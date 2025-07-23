#pragma once

// Standard includes
#include <string>
#include <functional>
#include <memory>

// MikoView core components
#include "mikoview/mikoapp.hpp"
#include "mikoview/mikoclient.hpp"
#include "mikoview/logger.hpp"

// MikoView GUI components
#include "mikoview/gui/platform_gui.hpp"

// MikoView JavaScript API
#include "mikoview/jsapi/invoke.hpp"

// Forward declarations
struct SDL_Window;

namespace MikoView {
    // Application configuration
    struct Config {
        std::string window_title = "MikoView";
        int window_width = 1200;
        int window_height = 800;
        bool debug_mode = false;
        int remote_debugging_port = 9222;
        std::string startup_url = "http://localhost:3000";
        bool start_hidden = true;  // Electron-style behavior
        int show_timeout_seconds = 10;
    };
    
    // Application state
    enum class State {
        Uninitialized,
        Initializing,
        Running,
        Shutting_Down,
        Shutdown
    };
    
    // Main application class
    class Application {
    public:
        Application();
        ~Application();
        
        // Core lifecycle
        bool Initialize(const Config& config = Config{});
        int Run();
        void Shutdown();
        
        // State management
        State GetState() const { return state_; }
        bool IsRunning() const { return state_ == State::Running; }
        
        // Window management
        void ShowWindow();
        void HideWindow();
        bool IsWindowVisible() const;
        void SetWindowTitle(const std::string& title);
        void SetWindowSize(int width, int height);
        
        // Browser management
        void LoadURL(const std::string& url);
        void Reload();
        bool HasBrowser() const;
        
        // Callbacks
        void SetReadyCallback(std::function<void()> callback);
        void SetCloseCallback(std::function<void()> callback);
        
        // Platform info
        static std::string GetPlatformName();
        static bool IsDarkModeSupported();
        static bool IsSystemDarkMode();
        
        // Access to underlying components
        CefRefPtr<SimpleClient> GetClient() const;
        CefRefPtr<SimpleApp> GetApp() const;
        SDL_Window* GetSDLWindow() const;
        
    private:
        // Internal implementation
        class Impl;
        std::unique_ptr<Impl> impl_;
        
        State state_;
        Config config_;
        
        // Non-copyable
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
    };
    
    // Convenience functions for simple usage
    namespace Quick {
        // Simple one-liner to run an application
        int RunApp(const std::string& url = "http://localhost:3000", 
                  const std::string& title = "MikoView",
                  bool debug = false);
        
        // Create and configure application
        std::unique_ptr<Application> CreateApp(const Config& config = Config{});
    }
    
    // Utility functions
    namespace Utils {
        // Logging (wraps Logger class)
        void LogInfo(const std::string& message);
        void LogWarning(const std::string& message);
        void LogError(const std::string& message);
        
        // File system helpers
        std::string GetExecutableDir();
        std::string GetResourcePath(const std::string& resource);
        bool FileExists(const std::string& path);
    }
    
    // Platform GUI utilities (wraps PlatformGUI namespace)
    namespace GUI {
        using WindowHandle = PlatformGUI::WindowHandle;
        
        bool InitializeDarkMode();
        bool IsDarkModeSupported();
        bool IsSystemDarkMode();
        bool EnableDarkModeForWindow(const WindowHandle& handle);
        void ApplyDarkTheme(const WindowHandle& handle);
        void RefreshWindowTheme(const WindowHandle& handle);
        WindowHandle GetNativeWindowHandle(SDL_Window* window);
    }
    
    // JavaScript API utilities (wraps JSApi)
    namespace JS {
        // Register JavaScript functions
        void RegisterInvokeHandler();
        
        // Execute JavaScript in the browser
        void ExecuteScript(const std::string& script);
        void ExecuteScriptInFrame(const std::string& script, const std::string& frame_name = "");
    }
}