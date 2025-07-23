#pragma once
#include <string>

class AppConfig {
public:
    static bool IsDebugMode();
    static std::string GetStartupUrl();
    
private:
    static bool ExtractAppZip(const std::string& zipPath, const std::string& extractPath);
};