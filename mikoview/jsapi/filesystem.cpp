#include "filesystem.hpp"
#include "../logger.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <algorithm>
#include <regex>

namespace MikoView {
namespace JSAPI {
namespace FileSystem {

// FileInfo implementation
std::string FileInfo::ToJSON() const {
    Json::Value root;
    root["name"] = name;
    root["path"] = path;
    root["extension"] = extension;
    root["size"] = static_cast<Json::Int64>(size);
    root["modified"] = static_cast<Json::Int64>(modified);
    root["created"] = static_cast<Json::Int64>(created);
    root["isDirectory"] = isDirectory;
    root["isFile"] = isFile;
    root["isSymlink"] = isSymlink;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

// DirectoryEntry implementation
std::string DirectoryEntry::ToJSON() const {
    Json::Value root;
    root["name"] = name;
    root["path"] = path;
    root["isDirectory"] = isDirectory;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

// ReadResult implementation
std::string ReadResult::ToJSON() const {
    Json::Value root;
    root["success"] = success;
    root["data"] = data;
    root["error"] = error;
    root["encoding"] = encoding;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

// WriteResult implementation
std::string WriteResult::ToJSON() const {
    Json::Value root;
    root["success"] = success;
    root["error"] = error;
    root["bytesWritten"] = static_cast<Json::Int64>(bytesWritten);
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

// FileSystemHandler implementation
void FileSystemHandler::RegisterHandlers() {
    auto* handler = InvokeHandler::GetInstance();
    
    // File operations
    handler->RegisterHandler("fs.readFile", HandleReadFile);
    handler->RegisterHandler("fs.writeFile", HandleWriteFile);
    handler->RegisterHandler("fs.appendFile", HandleAppendFile);
    handler->RegisterHandler("fs.deleteFile", HandleDeleteFile);
    handler->RegisterHandler("fs.copyFile", HandleCopyFile);
    handler->RegisterHandler("fs.moveFile", HandleMoveFile);
    
    // Directory operations
    handler->RegisterHandler("fs.readDir", HandleReadDir);
    handler->RegisterHandler("fs.createDir", HandleCreateDir);
    handler->RegisterHandler("fs.deleteDir", HandleDeleteDir);
    
    // File/Directory info
    handler->RegisterHandler("fs.getFileInfo", HandleGetFileInfo);
    handler->RegisterHandler("fs.exists", HandleExists);
    
    // Path operations
    handler->RegisterHandler("fs.resolvePath", HandleResolvePath);
    handler->RegisterHandler("fs.basename", HandleGetBasename);
    handler->RegisterHandler("fs.dirname", HandleGetDirname);
    handler->RegisterHandler("fs.extname", HandleGetExtname);
    handler->RegisterHandler("fs.joinPath", HandleJoinPath);
    
    Logger::Info("FileSystem handlers registered");
}

void FileSystemHandler::HandleReadFile(const InvokeRequest& request, InvokeResponse& response) {
    std::string path;
    std::string encoding = "utf8";
    
    if (!request.GetParam("path", path)) {
        response.SetError("Missing required parameter: path", 400);
        return;
    }
    
    request.GetParam("encoding", encoding);
    
    if (!IsPathSafe(path)) {
        response.SetError("Unsafe path", 403);
        return;
    }
    
    try {
        std::filesystem::path fsPath(path);
        if (!std::filesystem::exists(fsPath)) {
            response.SetError("File not found", 404);
            return;
        }
        
        if (!std::filesystem::is_regular_file(fsPath)) {
            response.SetError("Path is not a file", 400);
            return;
        }
        
        ReadResult result;
        result.encoding = encoding;
        
        if (encoding == "binary" || encoding == "base64") {
            std::ifstream file(fsPath, std::ios::binary);
            if (!file) {
                result.success = false;
                result.error = "Failed to open file";
            } else {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();
                
                if (encoding == "base64") {
                    // Base64 encode the content
                    // Implementation needed for base64 encoding
                    result.data = content; // Placeholder
                } else {
                    result.data = content;
                }
                result.success = true;
            }
        } else {
            std::ifstream file(fsPath);
            if (!file) {
                result.success = false;
                result.error = "Failed to open file";
            } else {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                result.data = buffer.str();
                result.success = true;
            }
        }
        
        response.SetSuccess(result.ToJSON());
    } catch (const std::exception& e) {
        response.SetError("File read error: " + std::string(e.what()), 500);
    }
}

void FileSystemHandler::HandleWriteFile(const InvokeRequest& request, InvokeResponse& response) {
    std::string path, data;
    std::string encoding = "utf8";
    bool createDirs = false;
    
    if (!request.GetParam("path", path) || !request.GetParam("data", data)) {
        response.SetError("Missing required parameters: path, data", 400);
        return;
    }
    
    request.GetParam("encoding", encoding);
    request.GetParam("createDirs", createDirs);
    
    if (!IsPathSafe(path)) {
        response.SetError("Unsafe path", 403);
        return;
    }
    
    try {
        std::filesystem::path fsPath(path);
        
        if (createDirs) {
            std::filesystem::create_directories(fsPath.parent_path());
        }
        
        WriteResult result;
        
        if (encoding == "binary" || encoding == "base64") {
            std::ofstream file(fsPath, std::ios::binary);
            if (!file) {
                result.success = false;
                result.error = "Failed to open file for writing";
            } else {
                std::string writeData = data;
                if (encoding == "base64") {
                    // Base64 decode the data
                    // Implementation needed for base64 decoding
                }
                file.write(writeData.c_str(), writeData.length());
                result.bytesWritten = writeData.length();
                result.success = true;
            }
        } else {
            std::ofstream file(fsPath);
            if (!file) {
                result.success = false;
                result.error = "Failed to open file for writing";
            } else {
                file << data;
                result.bytesWritten = data.length();
                result.success = true;
            }
        }
        
        response.SetSuccess(result.ToJSON());
    } catch (const std::exception& e) {
        response.SetError("File write error: " + std::string(e.what()), 500);
    }
}

void FileSystemHandler::HandleReadDir(const InvokeRequest& request, InvokeResponse& response) {
    std::string path;
    bool recursive = false;
    
    if (!request.GetParam("path", path)) {
        response.SetError("Missing required parameter: path", 400);
        return;
    }
    
    request.GetParam("recursive", recursive);
    
    if (!IsPathSafe(path)) {
        response.SetError("Unsafe path", 403);
        return;
    }
    
    try {
        std::filesystem::path fsPath(path);
        if (!std::filesystem::exists(fsPath)) {
            response.SetError("Directory not found", 404);
            return;
        }
        
        if (!std::filesystem::is_directory(fsPath)) {
            response.SetError("Path is not a directory", 400);
            return;
        }
        
        Json::Value entries(Json::arrayValue);
        
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(fsPath)) {
                DirectoryEntry dirEntry;
                dirEntry.name = entry.path().filename().string();
                dirEntry.path = entry.path().string();
                dirEntry.isDirectory = entry.is_directory();
                
                Json::Value entryJson;
                Json::Reader reader;
                reader.parse(dirEntry.ToJSON(), entryJson);
                entries.append(entryJson);
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(fsPath)) {
                DirectoryEntry dirEntry;
                dirEntry.name = entry.path().filename().string();
                dirEntry.path = entry.path().string();
                dirEntry.isDirectory = entry.is_directory();
                
                Json::Value entryJson;
                Json::Reader reader;
                reader.parse(dirEntry.ToJSON(), entryJson);
                entries.append(entryJson);
            }
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        response.SetSuccess(Json::writeString(builder, entries));
    } catch (const std::exception& e) {
        response.SetError("Directory read error: " + std::string(e.what()), 500);
    }
}

void FileSystemHandler::HandleExists(const InvokeRequest& request, InvokeResponse& response) {
    std::string path;
    
    if (!request.GetParam("path", path)) {
        response.SetError("Missing required parameter: path", 400);
        return;
    }
    
    if (!IsPathSafe(path)) {
        response.SetError("Unsafe path", 403);
        return;
    }
    
    try {
        bool exists = std::filesystem::exists(path);
        Json::Value result;
        result["exists"] = exists;
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        response.SetSuccess(Json::writeString(builder, result));
    } catch (const std::exception& e) {
        response.SetError("Path check error: " + std::string(e.what()), 500);
    }
}

// Utility functions
bool FileSystemHandler::IsPathSafe(const std::string& path) {
    // Basic path safety checks
    if (path.empty()) return false;
    
    // Check for dangerous patterns
    std::vector<std::string> dangerous = {"..", "~", "$"};
    for (const auto& pattern : dangerous) {
        if (path.find(pattern) != std::string::npos) {
            return false;
        }
    }
    
    // Additional platform-specific checks can be added here
    return true;
}

std::string FileSystemHandler::NormalizePath(const std::string& path) {
    try {
        return std::filesystem::canonical(path).string();
    } catch (...) {
        return path;
    }
}

// Additional handler implementations would go here...
// (HandleAppendFile, HandleDeleteFile, HandleCopyFile, etc.)

} // namespace FileSystem
} // namespace JSAPI
} // namespace MikoView