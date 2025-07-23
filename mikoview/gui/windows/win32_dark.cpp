#include "win32_dark.hpp"
#include <iostream>
#include <versionhelpers.h>

namespace Win32Dark {
    // Global state
    bool g_darkModeSupported = false;
    bool g_darkModeEnabled = false;
    
    // Function pointers for Windows 10+ APIs
    typedef BOOL(WINAPI* fnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
    typedef BOOL(WINAPI* fnShouldAppsUseDarkMode)(); // ordinal 132
    typedef BOOL(WINAPI* fnAllowDarkModeForWindow)(HWND, BOOL); // ordinal 133
    typedef BOOL(WINAPI* fnAllowDarkModeForApp)(BOOL); // ordinal 135
    typedef BOOL(WINAPI* fnFlushMenuThemes)(); // ordinal 136
    typedef BOOL(WINAPI* fnRefreshImmersiveColorPolicyState)(); // ordinal 104
    typedef BOOL(WINAPI* fnIsDarkModeAllowedForWindow)(HWND); // ordinal 137
    typedef BOOL(WINAPI* fnGetIsImmersiveColorUsingHighContrast)(int); // ordinal 106
    typedef HTHEME(WINAPI* fnOpenNcThemeData)(HWND, LPCWSTR); // ordinal 49
    
    // Function pointers
    static fnSetWindowCompositionAttribute _SetWindowCompositionAttribute = nullptr;
    static fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
    static fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
    static fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
    static fnFlushMenuThemes _FlushMenuThemes = nullptr;
    static fnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = nullptr;
    static fnIsDarkModeAllowedForWindow _IsDarkModeAllowedForWindow = nullptr;
    static fnGetIsImmersiveColorUsingHighContrast _GetIsImmersiveColorUsingHighContrast = nullptr;
    static fnOpenNcThemeData _OpenNcThemeData = nullptr;
    
    bool InitializeDarkMode() {
        // Check if we're on Windows 10 build 17763 or later
        if (!IsWindows10OrGreater()) {
            return false;
        }
        
        // Load user32.dll and uxtheme.dll
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        
        if (!hUser32 || !hUxtheme) {
            return false;
        }
        
        // Load function pointers
        _SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(
            GetProcAddress(hUser32, "SetWindowCompositionAttribute"));
        
        // Load uxtheme functions by ordinal (these are undocumented)
        _ShouldAppsUseDarkMode = reinterpret_cast<fnShouldAppsUseDarkMode>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
        _AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
        _AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));
        _FlushMenuThemes = reinterpret_cast<fnFlushMenuThemes>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136)));
        _RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
        _IsDarkModeAllowedForWindow = reinterpret_cast<fnIsDarkModeAllowedForWindow>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(137)));
        _GetIsImmersiveColorUsingHighContrast = reinterpret_cast<fnGetIsImmersiveColorUsingHighContrast>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(106)));
        _OpenNcThemeData = reinterpret_cast<fnOpenNcThemeData>(
            GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
        
        // Check if we have the minimum required functions
        g_darkModeSupported = (_SetWindowCompositionAttribute != nullptr) &&
                             (_AllowDarkModeForWindow != nullptr) &&
                             (_AllowDarkModeForApp != nullptr);
        
        if (g_darkModeSupported) {
            // Enable dark mode for the application
            if (_AllowDarkModeForApp) {
                _AllowDarkModeForApp(TRUE);
            }
            
            if (_RefreshImmersiveColorPolicyState) {
                _RefreshImmersiveColorPolicyState();
            }
            
            g_darkModeEnabled = true;
        }
        
        return g_darkModeSupported;
    }
    
    bool IsDarkModeSupported() {
        return g_darkModeSupported;
    }
    
    bool IsSystemDarkMode() {
        if (!g_darkModeSupported || !_ShouldAppsUseDarkMode) {
            return false;
        }
        
        return _ShouldAppsUseDarkMode();
    }
    
    bool EnableDarkModeForWindow(HWND hwnd) {
        if (!g_darkModeSupported || !hwnd) {
            return false;
        }
        
        if (_AllowDarkModeForWindow) {
            _AllowDarkModeForWindow(hwnd, TRUE);
        }
        
        return SetDarkTitleBar(hwnd, true);
    }
    
    bool SetWindowDarkMode(HWND hwnd, bool enable) {
        if (!g_darkModeSupported || !hwnd) {
            return false;
        }
        
        if (_AllowDarkModeForWindow) {
            _AllowDarkModeForWindow(hwnd, enable ? TRUE : FALSE);
        }
        
        return SetDarkTitleBar(hwnd, enable);
    }
    
    bool SetDarkTitleBar(HWND hwnd, bool dark) {
        if (!hwnd || !_SetWindowCompositionAttribute) {
            return false;
        }
        
        // For Windows 11 and newer builds of Windows 10
        BOOL darkMode = dark ? TRUE : FALSE;
        
        // Try the newer attribute first (Windows 11)
        WINDOWCOMPOSITIONATTRIBDATA data = {
            WCA_USEDARKMODECOLORS,
            &darkMode,
            sizeof(darkMode)
        };
        
        BOOL result = _SetWindowCompositionAttribute(hwnd, &data);
        
        // If that fails, try the older method
        if (!result) {
            // Use DWM API for older Windows 10 builds
            BOOL value = dark ? TRUE : FALSE;
            HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
            
            // Try alternative attribute for even older builds
            if (FAILED(hr)) {
                const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
                hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &value, sizeof(value));
            }
            
            result = SUCCEEDED(hr);
        }
        
        // Force window frame update
        if (result) {
            UpdateWindowFrame(hwnd);
        }
        
        return result != FALSE;
    }
    
    void ApplyDarkTheme(HWND hwnd) {
        if (!hwnd) return;
        
        // Enable dark mode for the window (safer approach)
        EnableDarkModeForWindow(hwnd);
        
        // Don't modify the window class - just refresh the frame
        UpdateWindowFrame(hwnd);
    }
    
    void RefreshTitleBarTheme(HWND hwnd) {
        if (!hwnd) return;
        
        // Check if system is in dark mode and apply accordingly
        bool systemDark = IsSystemDarkMode();
        SetDarkTitleBar(hwnd, systemDark);
        
        // Flush menu themes to ensure consistency
        if (_FlushMenuThemes) {
            _FlushMenuThemes();
        }
    }
    
    void UpdateWindowFrame(HWND hwnd) {
        if (!hwnd) return;
        
        // Force the window frame to redraw
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        
        // Send WM_THEMECHANGED to update the theme
        SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
    }
    
    LRESULT CALLBACK DarkModeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                                       UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
        switch (uMsg) {
            case WM_THEMECHANGED:
            case WM_SETTINGCHANGE:
                // Refresh dark mode when system theme changes
                RefreshTitleBarTheme(hwnd);
                break;
                
            case WM_DWMCOMPOSITIONCHANGED:
                // Handle DWM composition changes
                UpdateWindowFrame(hwnd);
                break;
                
            case WM_NCACTIVATE:
                // Handle non-client area activation
                if (g_darkModeSupported) {
                    // Let the default handler process it, then update our theme
                    LRESULT result = DefSubclassProc(hwnd, uMsg, wParam, lParam);
                    RefreshTitleBarTheme(hwnd);
                    return result;
                }
                break;
                
            case WM_NCDESTROY:
                // Clean up subclassing
                RemoveWindowSubclass(hwnd, DarkModeWindowProc, uIdSubclass);
                break;
        }
        
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }
}