#pragma once

#include "invoke.hpp"
#include <string>
#include <vector>
#include <memory>
#include <fstream>

namespace MikoView {
namespace JSAPI {
namespace FileSystem {

// File/Directory info structures
struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    size_t size;
    time_t modified;
    time_t created;
    bool isDirectory;
    bool isFile;
    bool isSymlink;
    
    std::string ToJSON() const;
};

struct DirectoryEntry {
    std::string name;
    std::string path;
    bool isDirectory;
    
    std::string ToJSON() const;
};

// File operation results
struct ReadResult {
    bool success;
    std::string data;
    std::string error;
    std::string encoding;
    
    std::string ToJSON() const;
};

struct WriteResult {
    bool success;
    std::string error;
    size_t bytesWritten;
    
    std::string ToJSON() const;
};

// Main filesystem handler class
class FileSystemHandler {
public:
    static void RegisterHandlers();
    
private:
    // File operations
    static void HandleReadFile(const InvokeRequest& request, InvokeResponse& response);
    static void HandleWriteFile(const InvokeRequest& request, InvokeResponse& response);
    static void HandleAppendFile(const InvokeRequest& request, InvokeResponse& response);
    static void HandleDeleteFile(const InvokeRequest& request, InvokeResponse& response);
    static void HandleCopyFile(const InvokeRequest& request, InvokeResponse& response);
    static void HandleMoveFile(const InvokeRequest& request, InvokeResponse& response);
    
    // Directory operations
    static void HandleReadDir(const InvokeRequest& request, InvokeResponse& response);
    static void HandleCreateDir(const InvokeRequest& request, InvokeResponse& response);
    static void HandleDeleteDir(const InvokeRequest& request, InvokeResponse& response);
    
    // File/Directory info
    static void HandleGetFileInfo(const InvokeRequest& request, InvokeResponse& response);
    static void HandleExists(const InvokeRequest& request, InvokeResponse& response);
    
    // Path operations
    static void HandleResolvePath(const InvokeRequest& request, InvokeResponse& response);
    static void HandleGetBasename(const InvokeRequest& request, InvokeResponse& response);
    static void HandleGetDirname(const InvokeRequest& request, InvokeResponse& response);
    static void HandleGetExtname(const InvokeRequest& request, InvokeResponse& response);
    static void HandleJoinPath(const InvokeRequest& request, InvokeResponse& response);
    
    // Utility functions
    static bool IsPathSafe(const std::string& path);
    static std::string NormalizePath(const std::string& path);
    static std::string GetMimeType(const std::string& extension);
    static std::string DetectEncoding(const std::vector<uint8_t>& data);
};

// File watcher (for future implementation)
class FileWatcher {
public:
    using WatchCallback = std::function<void(const std::string& path, const std::string& event)>;
    
    static void WatchFile(const std::string& path, WatchCallback callback);
    static void WatchDirectory(const std::string& path, WatchCallback callback, bool recursive = false);
    static void UnwatchPath(const std::string& path);
    static void UnwatchAll();
    
private:
    static std::map<std::string, WatchCallback> watchers_;
};

} // namespace FileSystem
} // namespace JSAPI
} // namespace MikoView