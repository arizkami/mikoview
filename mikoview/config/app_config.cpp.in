#include "app_config.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

// Static member definitions
bool AppConfig::assets_preloaded_ = false;
std::string AppConfig::preloaded_url_ = "";

bool AppConfig::IsDebugMode() {
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

bool AppConfig::PreloadAssets() {
    if (assets_preloaded_) {
        return true; // Already preloaded
    }
    
    if (IsDebugMode()) {
        // In debug mode, just set the URL - no preloading needed
        preloaded_url_ = "http://localhost:3000";
        assets_preloaded_ = true;
        return true;
    }
    
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path assetsPath = exePath / "assets";
    std::filesystem::path indexPath = assetsPath / "index.html";
    
    // Check if assets are already extracted
    if (std::filesystem::exists(indexPath)) {
        // Convert Windows path to proper file URL format
        std::string pathStr = indexPath.string();
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
        preloaded_url_ = "file:///" + pathStr;
        assets_preloaded_ = true;
        return true;
    }
    
    // Try to extract from app.zip
    std::filesystem::path zipPath = exePath / "assets" / "app.zip";
    if (std::filesystem::exists(zipPath)) {
        if (ExtractAppZip(zipPath.string(), assetsPath.string())) {
            if (std::filesystem::exists(indexPath)) {
                preloaded_url_ = "app://index.html";
                assets_preloaded_ = true;
                return true;
            }
        }
    }
    
    // Fallback error page
    preloaded_url_ = "data:text/html,<html><body><h1>Assets not found</h1><p>Please ensure app.zip exists in the assets directory</p></body></html>";
    assets_preloaded_ = true;
    return false;
}

std::string AppConfig::GetPreloadedUrl() {
    if (assets_preloaded_) {
        return preloaded_url_;
    }
    return GetStartupUrl(); // Fallback to original method
}

std::string AppConfig::GetStartupUrl() {
    if (IsDebugMode()) {
        return "http://localhost:3000";
    } else {
        std::filesystem::path exePath = std::filesystem::current_path();
        std::filesystem::path assetsPath = exePath / "assets";
        std::filesystem::path indexPath = assetsPath / "index.html";
        
        // Check if assets are already extracted
        if (std::filesystem::exists(indexPath)) {
            // Convert Windows path to proper file URL format
            std::string pathStr = indexPath.string();
            // Replace backslashes with forward slashes for file URLs
            std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
            return "file:///" + pathStr;
        }
        
        // Try to extract from app.zip
        std::filesystem::path zipPath = exePath / "assets" / "app.zip";
        if (std::filesystem::exists(zipPath)) {
            if (ExtractAppZip(zipPath.string(), assetsPath.string())) {
                if (std::filesystem::exists(indexPath)) {
                    return "app://index.html";
                }
            }
        }
        
        return "data:text/html,<html><body><h1>Assets not found</h1><p>Please ensure app.zip exists in the assets directory</p></body></html>";
    }
}

bool AppConfig::ExtractAppZip(const std::string& zipPath, const std::string& extractPath) {
    try {
        // Create extraction directory if it doesn't exist
        std::filesystem::create_directories(extractPath);
        
        // Use system command to extract zip (Windows built-in)
        std::string command = "powershell -command \"Expand-Archive -Path '" + zipPath + "' -DestinationPath '" + extractPath + "' -Force\"";
        int result = system(command.c_str());
        
        return result == 0;
    } catch (const std::exception& e) {
        std::cerr << "Error extracting zip: " << e.what() << std::endl;
        return false;
    }
}