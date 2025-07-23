#include "x11_dark.hpp"

#ifdef __linux__
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fstream>

namespace X11Dark {
    // Global state
    bool g_darkModeSupported = false;
    bool g_darkModeEnabled = false;
    
    // Atoms for window properties
    static Atom _NET_WM_NAME = None;
    static Atom _GTK_THEME_VARIANT = None;
    static Atom UTF8_STRING = None;
    
    bool InitializeDarkMode() {
        // Check if we're running under X11
        const char* display_env = getenv("DISPLAY");
        if (!display_env) {
            return false; // Not running under X11
        }
        
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            return false;
        }
        
        // Initialize atoms
        _NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", False);
        _GTK_THEME_VARIANT = XInternAtom(display, "_GTK_THEME_VARIANT", False);
        UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
        
        g_darkModeSupported = (_NET_WM_NAME != None && _GTK_THEME_VARIANT != None);
        
        if (g_darkModeSupported) {
            g_darkModeEnabled = IsSystemDarkMode();
        }
        
        XCloseDisplay(display);
        return g_darkModeSupported;
    }
    
    bool IsDarkModeSupported() {
        return g_darkModeSupported;
    }
    
    bool IsSystemDarkMode() {
        // Check GTK theme preference
        const char* gtk_theme = getenv("GTK_THEME");
        if (gtk_theme && strstr(gtk_theme, "dark")) {
            return true;
        }
        
        // Check gsettings for GNOME
        FILE* pipe = popen("gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                pclose(pipe);
                return strstr(buffer, "dark") != nullptr;
            }
            pclose(pipe);
        }
        
        // Check for KDE dark theme
        pipe = popen("kreadconfig5 --group General --key ColorScheme 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                pclose(pipe);
                return strstr(buffer, "Dark") != nullptr || strstr(buffer, "dark") != nullptr;
            }
            pclose(pipe);
        }
        
        return false;
    }
    
    bool EnableDarkModeForWindow(Display* display, Window window) {
        if (!g_darkModeSupported || !display || window == None) {
            return false;
        }
        
        return SetDarkTitleBar(display, window, true);
    }
    
    bool SetWindowDarkMode(Display* display, Window window, bool enable) {
        if (!g_darkModeSupported || !display || window == None) {
            return false;
        }
        
        return SetDarkTitleBar(display, window, enable);
    }
    
    bool SetDarkTitleBar(Display* display, Window window, bool dark) {
        if (!display || window == None || _GTK_THEME_VARIANT == None) {
            return false;
        }
        
        const char* variant = dark ? "dark" : "light";
        
        XChangeProperty(display, window, _GTK_THEME_VARIANT, UTF8_STRING, 8,
                       PropModeReplace, (unsigned char*)variant, strlen(variant));
        
        XFlush(display);
        return true;
    }
    
    void ApplyDarkTheme(Display* display, Window window) {
        if (!display || window == None) return;
        
        // Enable dark mode for the window
        EnableDarkModeForWindow(display, window);
        
        // Update window properties
        UpdateWindowProperties(display, window);
    }
    
    void RefreshWindowTheme(Display* display, Window window) {
        if (!display || window == None) return;
        
        // Check if system is in dark mode and apply accordingly
        bool systemDark = IsSystemDarkMode();
        SetDarkTitleBar(display, window, systemDark);
    }
    
    void UpdateWindowProperties(Display* display, Window window) {
        if (!display || window == None) return;
        
        // Force window manager to update the window
        XMapWindow(display, window);
        XFlush(display);
    }
}

#endif // __linux__