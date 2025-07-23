#pragma once
#include <string>

class AppConfig {
public:
    static bool IsDebugMode();
    static std::string GetStartupUrl();
    static bool PreloadAssets(); // New preload function
    static std::string GetPreloadedUrl(); // Get preloaded URL
    
private:
    static bool ExtractAppZip(const std::string& zipPath, const std::string& extractPath);
    static bool assets_preloaded_; // Track preload status
    static std::string preloaded_url_; // Cache preloaded URL
};