#include "mikoview.hpp"

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #ifdef GetFirstChild
    #undef GetFirstChild
    #endif
    #ifdef GetNextSibling
    #undef GetNextSibling
    #endif
    #ifdef GetPrevSibling
    #undef GetPrevSibling
    #endif
    #ifdef GetParent
    #undef GetParent
    #endif
#endif

// SDL and CEF includes
#include <SDL.h>
#include <SDL_syswm.h>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

// Standard includes
#include <filesystem>
#include <chrono>
#include <iostream>

namespace MikoView {
    
    // Internal implementation class
    class Application::Impl {
    public:
        SDL_Window* sdl_window = nullptr;
        PlatformGUI::WindowHandle native_handle;
        CefRefPtr<SimpleClient> client;
        CefRefPtr<SimpleApp> app;
        
        bool running = true;
        bool window_shown = false;
        std::chrono::steady_clock::time_point start_time;
        
        std::function<void()> ready_callback;
        std::function<void()> close_callback;
        
        void HandleSDLEvents(Application* app_instance) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        running = false;
                        if (client) {
                            client->CloseAllBrowsers(false);
                        }
                        if (app_instance && close_callback) {
                            close_callback();
                        }
                        break;
                        
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                            if (client && client->HasBrowsers()) {
                                CefRefPtr<CefBrowser> browser = client->GetFirstBrowser();
                                if (browser) {
#ifdef _WIN32
                                    HWND cef_hwnd = browser->GetHost()->GetWindowHandle();
                                    if (cef_hwnd) {
                                        int width = event.window.data1;
                                        int height = event.window.data2;
                                        SetWindowPos(cef_hwnd, nullptr, 0, 0, width, height,
                                                   SWP_NOZORDER | SWP_NOACTIVATE);
                                    }
#elif defined(__linux__)
                                    // On Linux, CEF handles window resizing automatically with X11
#endif
                                }
                            }
                        }
                        break;
                }
            }
        }
        
        void ShowWindowWhenReady() {
            if (!window_shown && sdl_window) {
                Utils::LogInfo("✨ Showing window - content is ready!");
                SDL_ShowWindow(sdl_window);
                SDL_RaiseWindow(sdl_window);
                window_shown = true;
                
                if (ready_callback) {
                    ready_callback();
                }
            }
        }
    };
    
    // Application implementation
    Application::Application() : impl_(std::make_unique<Impl>()), state_(State::Uninitialized) {
    }
    
    Application::~Application() {
        if (state_ != State::Shutdown) {
            Shutdown();
        }
    }
    
    bool Application::Initialize(const Config& config) {
        if (state_ != State::Uninitialized) {
            Utils::LogError("Application already initialized");
            return false;
        }
        
        state_ = State::Initializing;
        config_ = config;
        
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            Utils::LogError("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
            state_ = State::Uninitialized;
            return false;
        }
        
        // Initialize platform-specific dark mode support
        GUI::InitializeDarkMode();
        
        // Create SDL window
        Uint32 window_flags = SDL_WINDOW_RESIZABLE;
        if (config_.start_hidden) {
            window_flags |= SDL_WINDOW_HIDDEN;
        }
        
        impl_->sdl_window = SDL_CreateWindow(
            config_.window_title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config_.window_width,
            config_.window_height,
            window_flags
        );
        
        if (!impl_->sdl_window) {
            Utils::LogError("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
            SDL_Quit();
            state_ = State::Uninitialized;
            return false;
        }
        
        // Get native window handle
        impl_->native_handle = GUI::GetNativeWindowHandle(impl_->sdl_window);
#ifdef _WIN32
        if (!impl_->native_handle) {
#else
        if (!impl_->native_handle.isValid()) {
#endif
            Utils::LogError("Could not get native window handle!");
            SDL_DestroyWindow(impl_->sdl_window);
            SDL_Quit();
            state_ = State::Uninitialized;
            return false;
        }
        
        // Apply dark theme if supported
        if (GUI::IsDarkModeSupported()) {
            GUI::ApplyDarkTheme(impl_->native_handle);
        }
        
        // Initialize CEF
        void* sandbox_info = nullptr;
#ifdef _WIN32
        CefMainArgs main_args(GetModuleHandle(nullptr));
#else
        // For Linux, we need to get argc/argv from somewhere
        // This is a limitation - in a real app, you'd pass these from main()
        CefMainArgs main_args(0, nullptr);
#endif
        
        CefSettings settings;
        settings.no_sandbox = true;
        settings.multi_threaded_message_loop = false;
        settings.background_color = 0xFFFFFFFF;
        
        if (config_.debug_mode) {
            settings.remote_debugging_port = config_.remote_debugging_port;
            settings.log_severity = LOGSEVERITY_INFO;
        } else {
            settings.log_severity = LOGSEVERITY_WARNING;
        }
        
        impl_->app = new SimpleApp();
        if (!CefInitialize(main_args, settings, impl_->app.get(), sandbox_info)) {
            Utils::LogError("Failed to initialize CEF");
            SDL_DestroyWindow(impl_->sdl_window);
            SDL_Quit();
            state_ = State::Uninitialized;
            return false;
        }
        
        // Create CEF browser
        CefWindowInfo window_info;
        int width, height;
        SDL_GetWindowSize(impl_->sdl_window, &width, &height);
        
        CefRect cef_rect(0, 0, width, height);
#ifdef _WIN32
        window_info.SetAsChild(impl_->native_handle, cef_rect);
#else
        window_info.SetAsChild(impl_->native_handle.window, cef_rect);
#endif
        
        CefBrowserSettings browser_settings;
        browser_settings.local_storage = STATE_ENABLED;
        
        impl_->client = new SimpleClient();
        
        // Set callback to show window when content is ready
        impl_->client->SetReadyCallback([this]() {
            impl_->ShowWindowWhenReady();
        });
        
        CefBrowserHost::CreateBrowser(window_info, impl_->client, config_.startup_url, browser_settings, nullptr, nullptr);
        
        impl_->start_time = std::chrono::steady_clock::now();
        state_ = State::Running;
        
        // Register JavaScript API handlers
        JS::RegisterInvokeHandler();
        
        // Log startup information
        Utils::LogInfo("=== MikoView Framework Initialized ===");
        Utils::LogInfo("Mode: " + std::string(config_.debug_mode ? "DEBUG" : "RELEASE"));
        Utils::LogInfo("Platform: " + GetPlatformName());
        Utils::LogInfo("URL: " + config_.startup_url);
        if (config_.start_hidden) {
            Utils::LogInfo("🔄 Window hidden until content loads...");
        }
        if (config_.debug_mode) {
            Utils::LogInfo("Remote debugging: http://localhost:" + std::to_string(config_.remote_debugging_port));
        }
        Utils::LogInfo("======================================");
        
        return true;
    }
    
    int Application::Run() {
        if (state_ != State::Running) {
            Utils::LogError("Application not properly initialized");
            return -1;
        }
        
        const auto timeout_duration = std::chrono::seconds(config_.show_timeout_seconds);
        
        // Main loop
        while (impl_->running && state_ == State::Running) {
            impl_->HandleSDLEvents(this);
            CefDoMessageLoopWork();
            
            // Timeout fallback for showing window
            if (config_.start_hidden && !impl_->window_shown) {
                auto current_time = std::chrono::steady_clock::now();
                if (current_time - impl_->start_time > timeout_duration) {
                    Utils::LogInfo("⏰ Timeout reached - showing window anyway");
                    impl_->ShowWindowWhenReady();
                }
            }
            
            SDL_Delay(1); // Small delay to prevent 100% CPU usage
        }
        
        return 0;
    }
    
    void Application::Shutdown() {
        if (state_ == State::Shutdown) {
            return;
        }
        
        state_ = State::Shutting_Down;
        
        if (impl_->client) {
            impl_->client->CloseAllBrowsers(true);
        }
        
        CefShutdown();
        
        if (impl_->sdl_window) {
            SDL_DestroyWindow(impl_->sdl_window);
            impl_->sdl_window = nullptr;
        }
        
        SDL_Quit();
        state_ = State::Shutdown;
        
        Utils::LogInfo("MikoView application shutdown complete");
    }
    
    void Application::ShowWindow() {
        if (impl_->sdl_window && !impl_->window_shown) {
            impl_->ShowWindowWhenReady();
        }
    }
    
    void Application::HideWindow() {
        if (impl_->sdl_window) {
            SDL_HideWindow(impl_->sdl_window);
            impl_->window_shown = false;
        }
    }
    
    bool Application::IsWindowVisible() const {
        return impl_->window_shown;
    }
    
    void Application::SetWindowTitle(const std::string& title) {
        if (impl_->sdl_window) {
            SDL_SetWindowTitle(impl_->sdl_window, title.c_str());
        }
    }
    
    void Application::SetWindowSize(int width, int height) {
        if (impl_->sdl_window) {
            SDL_SetWindowSize(impl_->sdl_window, width, height);
        }
    }
    
    void Application::LoadURL(const std::string& url) {
        if (impl_->client && impl_->client->HasBrowsers()) {
            CefRefPtr<CefBrowser> browser = impl_->client->GetFirstBrowser();
            if (browser) {
                browser->GetMainFrame()->LoadURL(url);
            }
        }
    }
    
    void Application::Reload() {
        if (impl_->client && impl_->client->HasBrowsers()) {
            CefRefPtr<CefBrowser> browser = impl_->client->GetFirstBrowser();
            if (browser) {
                browser->Reload();
            }
        }
    }
    
    bool Application::HasBrowser() const {
        return impl_->client && impl_->client->HasBrowsers();
    }
    
    void Application::SetReadyCallback(std::function<void()> callback) {
        impl_->ready_callback = callback;
    }
    
    void Application::SetCloseCallback(std::function<void()> callback) {
        impl_->close_callback = callback;
    }
    
    CefRefPtr<SimpleClient> Application::GetClient() const {
        return impl_->client;
    }
    
    CefRefPtr<SimpleApp> Application::GetApp() const {
        return impl_->app;
    }
    
    SDL_Window* Application::GetSDLWindow() const {
        return impl_->sdl_window;
    }
    
    std::string Application::GetPlatformName() {
#ifdef _WIN32
        return "Windows";
#elif defined(__linux__)
        return "Linux";
#elif defined(__APPLE__)
        return "macOS";
#else
        return "Unknown";
#endif
    }
    
    bool Application::IsDarkModeSupported() {
        return GUI::IsDarkModeSupported();
    }
    
    bool Application::IsSystemDarkMode() {
        return GUI::IsSystemDarkMode();
    }
    
    // Quick namespace implementation
    namespace Quick {
        int RunApp(const std::string& url, const std::string& title, bool debug) {
            Config config;
            config.startup_url = url;
            config.window_title = title;
            config.debug_mode = debug;
            
            auto app = CreateApp(config);
            if (!app) {
                return -1;
            }
            
            return app->Run();
        }
        
        std::unique_ptr<Application> CreateApp(const Config& config) {
            auto app = std::make_unique<Application>();
            if (!app->Initialize(config)) {
                return nullptr;
            }
            return app;
        }
    }
    
    // Utils namespace implementation
    namespace Utils {
        void LogInfo(const std::string& message) {
            Logger::LogMessage("[INFO] " + message);
        }
        
        void LogWarning(const std::string& message) {
            Logger::LogMessage("[WARNING] " + message);
        }
        
        void LogError(const std::string& message) {
            Logger::LogMessage("[ERROR] " + message);
        }
        
        std::string GetExecutableDir() {
            return std::filesystem::current_path().string();
        }
        
        std::string GetResourcePath(const std::string& resource) {
            return std::filesystem::path(GetExecutableDir()) / resource;
        }
        
        bool FileExists(const std::string& path) {
            return std::filesystem::exists(path);
        }
    }
    
    // GUI namespace implementation (wraps PlatformGUI)
    namespace GUI {
        bool InitializeDarkMode() {
            return PlatformGUI::InitializeDarkMode();
        }
        
        bool IsDarkModeSupported() {
            return PlatformGUI::IsDarkModeSupported();
        }
        
        bool IsSystemDarkMode() {
            return PlatformGUI::IsSystemDarkMode();
        }
        
        bool EnableDarkModeForWindow(const WindowHandle& handle) {
            return PlatformGUI::EnableDarkModeForWindow(handle);
        }
        
        void ApplyDarkTheme(const WindowHandle& handle) {
            PlatformGUI::ApplyDarkTheme(handle);
        }
        
        void RefreshWindowTheme(const WindowHandle& handle) {
            PlatformGUI::RefreshWindowTheme(handle);
        }
        
        WindowHandle GetNativeWindowHandle(SDL_Window* window) {
            return PlatformGUI::GetNativeWindowHandle(window);
        }
    }
    
    // JS namespace implementation (wraps JSApi)
    namespace JS {
        void RegisterInvokeHandler() {
            // This would integrate with the invoke.hpp functionality
            // Implementation depends on the specific JSApi design
        }
        
        void ExecuteScript(const std::string& script) {
            // Implementation would execute JavaScript in the main frame
        }
        
        void ExecuteScriptInFrame(const std::string& script, const std::string& frame_name) {
            // Implementation would execute JavaScript in a specific frame
        }
    }
}