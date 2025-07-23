// Platform-specific includes
#ifdef _WIN32
    // Remove the manual defines since CEF already defines them on command line
    // #define WIN32_LEAN_AND_MEAN
    // #define NOMINMAX
    #include <windows.h>
    
    // Undefine Windows macros that conflict with CEF
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

#include <SDL.h>
#include <SDL_syswm.h>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include <filesystem>
#include <functional>
#include <chrono>

// Local includes
#include "app_config.hpp"
#include <mikoview/logger.hpp>
#include <mikoview/mikoclient.hpp>
#include <mikoview/mikoapp.hpp>
#include <mikoview/gui/platform_gui.hpp>

// Global variables
CefRefPtr<SimpleClient> g_client;
SDL_Window* g_sdl_window = nullptr;
PlatformGUI::WindowHandle g_native_handle;
bool g_running = true;
bool g_window_shown = false;

// Function to show the window when content is ready
void ShowWindowWhenReady() {
    if (!g_window_shown && g_sdl_window) {
        Logger::LogMessage("✨ Showing window - content is ready!");
        SDL_ShowWindow(g_sdl_window);
        SDL_RaiseWindow(g_sdl_window);
        g_window_shown = true;
    }
}

// Handle SDL events
void HandleSDLEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                g_running = false;
                if (g_client) {
                    g_client->CloseAllBrowsers(false);
                }
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    if (g_client && g_client->HasBrowsers()) {
                        CefRefPtr<CefBrowser> browser = g_client->GetFirstBrowser();
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
                            // The browser will resize with the parent window
#endif
                        }
                    }
                }
                break;
        }
    }
}

// Cross-platform main function
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char* argv[]) {
#endif
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        Logger::LogMessage("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        return 1;
    }

    void* sandbox_info = nullptr;
#ifdef _WIN32
    CefMainArgs main_args(GetModuleHandle(nullptr));
#else
    CefMainArgs main_args(argc, argv);
#endif

    // CEF sub-process check
    int exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
    if (exit_code >= 0) {
        return exit_code;
    }

    // Initialize platform-specific dark mode support
    PlatformGUI::InitializeDarkMode();

    // Create SDL window (HIDDEN initially like Electron)
    std::string windowTitle = AppConfig::IsDebugMode() ? 
        "MikoView - Development Mode" : "MikoView - Release Mode";
    
    g_sdl_window = SDL_CreateWindow(
        windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200, 800,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN  // Start HIDDEN
    );

    if (!g_sdl_window) {
        Logger::LogMessage("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        SDL_Quit();
        return 1;
    }

    // Get the native window handle
    g_native_handle = PlatformGUI::GetNativeWindowHandle(g_sdl_window);
#ifdef _WIN32
    if (!g_native_handle) {
#else
    if (!g_native_handle.isValid()) {
#endif
        Logger::LogMessage("Could not get native window handle!");
        SDL_DestroyWindow(g_sdl_window);
        SDL_Quit();
        return 1;
    }

    // Apply dark theme if supported
    if (PlatformGUI::IsDarkModeSupported()) {
        PlatformGUI::ApplyDarkTheme(g_native_handle);
    }

    // CEF settings
    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = false;
    settings.background_color = 0xFFFFFFFF; // White background for faster rendering

    if (AppConfig::IsDebugMode()) {
        settings.remote_debugging_port = 9222;
        settings.log_severity = LOGSEVERITY_INFO;
    } else {
        settings.log_severity = LOGSEVERITY_WARNING;
    }

    CefRefPtr<SimpleApp> app(new SimpleApp);
    CefInitialize(main_args, settings, app.get(), sandbox_info);

    // Create CEF browser
    CefWindowInfo window_info;
    int width, height;
    SDL_GetWindowSize(g_sdl_window, &width, &height);

    CefRect cef_rect(0, 0, width, height);
#ifdef _WIN32
    window_info.SetAsChild(g_native_handle, cef_rect);
#else
    window_info.SetAsChild(g_native_handle.window, cef_rect);
#endif

    CefBrowserSettings browser_settings;
    browser_settings.local_storage = STATE_ENABLED;

    g_client = new SimpleClient();
    
    // Set callback to show window when content is ready
    g_client->SetReadyCallback(ShowWindowWhenReady);
    
    std::string startupUrl = AppConfig::GetStartupUrl();
    
    CefBrowserHost::CreateBrowser(window_info, g_client, startupUrl, browser_settings, nullptr, nullptr);

    // Log startup information
    Logger::LogMessage("=== MikoView CEF + SDL Application [ELECTRON-STYLE] ===");
    Logger::LogMessage("Mode: " + std::string(AppConfig::IsDebugMode() ? "DEBUG" : "RELEASE"));
    Logger::LogMessage("Platform: " + std::string(
#ifdef _WIN32
        "Windows"
#elif defined(__linux__)
        "Linux"
#else
        "Unknown"
#endif
    ));
    Logger::LogMessage("URL: " + startupUrl);
    Logger::LogMessage("🔄 Window hidden until content loads (like Electron)...");
    if (AppConfig::IsDebugMode()) {
        Logger::LogMessage("Remote debugging: http://localhost:9222");
        Logger::LogMessage("Make sure React dev server is running: cd renderer && bun run dev");
    }
    Logger::LogMessage("======================================");

    // Timeout fallback - show window after 10 seconds even if not ready
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout_duration = std::chrono::seconds(10);

    // Main loop
    while (g_running) {
        HandleSDLEvents();
        CefDoMessageLoopWork();
        
        // Timeout fallback
        if (!g_window_shown) {
            auto current_time = std::chrono::steady_clock::now();
            if (current_time - start_time > timeout_duration) {
                Logger::LogMessage("⏰ Timeout reached - showing window anyway");
                ShowWindowWhenReady();
            }
        }
        
        SDL_Delay(1); // Small delay to prevent 100% CPU usage
    }

    // Cleanup
    CefShutdown();
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();

    return 0;
}