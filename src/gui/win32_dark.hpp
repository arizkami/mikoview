#pragma once
#include <windows.h>
#include <dwmapi.h>
#include <uxtheme.h>

// Link required libraries
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

namespace Win32Dark {
    // Dark theme constants
    constexpr COLORREF DARK_BACKGROUND = RGB(32, 32, 32);
    constexpr COLORREF DARK_TEXT = RGB(255, 255, 255);
    constexpr COLORREF DARK_BORDER = RGB(64, 64, 64);
    constexpr COLORREF DARK_TITLEBAR = RGB(45, 45, 45);
    
    // Windows 10/11 dark mode attributes
    enum WINDOWCOMPOSITIONATTRIB {
        WCA_UNDEFINED = 0,
        WCA_NCRENDERING_ENABLED = 1,
        WCA_NCRENDERING_POLICY = 2,
        WCA_TRANSITIONS_FORCEDISABLED = 3,
        WCA_ALLOW_NCPAINT = 4,
        WCA_CAPTION_BUTTON_BOUNDS = 5,
        WCA_NONCLIENT_RTL_LAYOUT = 6,
        WCA_FORCE_ICONIC_REPRESENTATION = 7,
        WCA_EXTENDED_FRAME_BOUNDS = 8,
        WCA_HAS_ICONIC_BITMAP = 9,
        WCA_THEME_ATTRIBUTES = 10,
        WCA_NCRENDERING_EXILED = 11,
        WCA_NCADORNMENTINFO = 12,
        WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
        WCA_VIDEO_OVERLAY_ACTIVE = 14,
        WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
        WCA_DISALLOW_PEEK = 16,
        WCA_CLOAK = 17,
        WCA_CLOAKED = 18,
        WCA_ACCENT_POLICY = 19,
        WCA_FREEZE_REPRESENTATION = 20,
        WCA_EVER_UNCLOAKED = 21,
        WCA_VISUAL_OWNER = 22,
        WCA_HOLOGRAPHIC = 23,
        WCA_EXCLUDED_FROM_DDA = 24,
        WCA_PASSIVEUPDATEMODE = 25,
        WCA_USEDARKMODECOLORS = 26,
        WCA_LAST = 27
    };
    
    struct WINDOWCOMPOSITIONATTRIBDATA {
        WINDOWCOMPOSITIONATTRIB Attrib;
        PVOID pvData;
        SIZE_T cbData;
    };
    
    // Function declarations
    bool InitializeDarkMode();
    bool IsDarkModeSupported();
    bool IsSystemDarkMode();
    bool EnableDarkModeForWindow(HWND hwnd);
    bool SetWindowDarkMode(HWND hwnd, bool enable);
    void ApplyDarkTheme(HWND hwnd);
    void RefreshTitleBarTheme(HWND hwnd);
    
    // Window procedure for dark theme handling
    LRESULT CALLBACK DarkModeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    
    // Utility functions
    bool SetDarkTitleBar(HWND hwnd, bool dark);
    void UpdateWindowFrame(HWND hwnd);
    
    // Internal function pointers (loaded dynamically)
    extern bool g_darkModeSupported;
    extern bool g_darkModeEnabled;
}