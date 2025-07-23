#pragma once

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>

namespace X11Dark {
    // Dark theme constants
    constexpr unsigned long DARK_BACKGROUND = 0x202020;  // RGB(32, 32, 32)
    constexpr unsigned long DARK_TEXT = 0xFFFFFF;        // RGB(255, 255, 255)
    constexpr unsigned long DARK_BORDER = 0x404040;      // RGB(64, 64, 64)
    
    // Function declarations
    bool InitializeDarkMode();
    bool IsDarkModeSupported();
    bool IsSystemDarkMode();
    bool EnableDarkModeForWindow(Display* display, Window window);
    bool SetWindowDarkMode(Display* display, Window window, bool enable);
    void ApplyDarkTheme(Display* display, Window window);
    void RefreshWindowTheme(Display* display, Window window);
    
    // Utility functions
    bool SetDarkTitleBar(Display* display, Window window, bool dark);
    void UpdateWindowProperties(Display* display, Window window);
    
    // Global state
    extern bool g_darkModeSupported;
    extern bool g_darkModeEnabled;
}

#endif // __linux__