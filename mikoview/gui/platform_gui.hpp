#pragma once

#ifdef _WIN32
    #include "windows/win32_dark.hpp"
    #include <windows.h>
    #include <SDL_syswm.h>
    
    namespace PlatformGUI {
        using WindowHandle = HWND;
        
        inline bool InitializeDarkMode() {
            return Win32Dark::InitializeDarkMode();
        }
        
        inline bool IsDarkModeSupported() {
            return Win32Dark::IsDarkModeSupported();
        }
        
        inline bool IsSystemDarkMode() {
            return Win32Dark::IsSystemDarkMode();
        }
        
        inline bool EnableDarkModeForWindow(WindowHandle hwnd) {
            return Win32Dark::EnableDarkModeForWindow(hwnd);
        }
        
        inline void ApplyDarkTheme(WindowHandle hwnd) {
            Win32Dark::ApplyDarkTheme(hwnd);
        }
        
        inline void RefreshWindowTheme(WindowHandle hwnd) {
            Win32Dark::RefreshTitleBarTheme(hwnd);
        }
        
        inline WindowHandle GetNativeWindowHandle(SDL_Window* window) {
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            if (SDL_GetWindowWMInfo(window, &wmInfo)) {
                return wmInfo.info.win.window;
            }
            return nullptr;
        }
    }
    
#elif defined(__linux__)
    #include "linux/x11_dark.hpp"
    #include <X11/Xlib.h>
    #include <SDL_syswm.h>
    
    namespace PlatformGUI {
        struct WindowHandle {
            Display* display;
            Window window;
            
            WindowHandle() : display(nullptr), window(None) {}
            WindowHandle(Display* d, Window w) : display(d), window(w) {}
            
            bool isValid() const { return display != nullptr && window != None; }
        };
        
        inline bool InitializeDarkMode() {
            return X11Dark::InitializeDarkMode();
        }
        
        inline bool IsDarkModeSupported() {
            return X11Dark::IsDarkModeSupported();
        }
        
        inline bool IsSystemDarkMode() {
            return X11Dark::IsSystemDarkMode();
        }
        
        inline bool EnableDarkModeForWindow(const WindowHandle& handle) {
            if (!handle.isValid()) return false;
            return X11Dark::EnableDarkModeForWindow(handle.display, handle.window);
        }
        
        inline void ApplyDarkTheme(const WindowHandle& handle) {
            if (!handle.isValid()) return;
            X11Dark::ApplyDarkTheme(handle.display, handle.window);
        }
        
        inline void RefreshWindowTheme(const WindowHandle& handle) {
            if (!handle.isValid()) return;
            X11Dark::RefreshWindowTheme(handle.display, handle.window);
        }
        
        inline WindowHandle GetNativeWindowHandle(SDL_Window* window) {
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            if (SDL_GetWindowWMInfo(window, &wmInfo)) {
                return WindowHandle(wmInfo.info.x11.display, wmInfo.info.x11.window);
            }
            return WindowHandle();
        }
    }
    
#else
    // Fallback for unsupported platforms
    namespace PlatformGUI {
        using WindowHandle = void*;
        
        inline bool InitializeDarkMode() { return false; }
        inline bool IsDarkModeSupported() { return false; }
        inline bool IsSystemDarkMode() { return false; }
        inline bool EnableDarkModeForWindow(WindowHandle) { return false; }
        inline void ApplyDarkTheme(WindowHandle) {}
        inline void RefreshWindowTheme(WindowHandle) {}
        inline WindowHandle GetNativeWindowHandle(SDL_Window*) { return nullptr; }
    }
#endif