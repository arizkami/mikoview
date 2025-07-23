# Examples

## Basic File Manager

A simple file manager application demonstrating file system operations.

### C++ Code

```cpp
#include "mikoview.hpp"
#include <json/json.h>

class FileManagerApp : public MikoView::Application {
public:
    bool Initialize() override {
        if (!MikoView::Application::Initialize()) {
            return false;
        }
        
        // Register file operations
        RegisterHandler("file.browse", [this](const std::string& data) {
            return HandleBrowse(data);
        });
        
        RegisterHandler("file.open", [this](const std::string& data) {
            return HandleOpenFile(data);
        });
        
        return true;
    }
    
private:
    std::string HandleBrowse(const std::string& data) {
        Json::Value request;
        Json::Reader reader;
        if (!reader.parse(data, request)) {
            return "{\"error\": \"Invalid JSON\"}";
        }
        
        std::string path = request["path"].asString();
        
        // Use built-in file system API
        Json::Value response;
        response["path"] = path;
        response["success"] = true;
        
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, response);
    }
    
    std::string HandleOpenFile(const std::string& data) {
        // Implementation for opening files
        return "{\"success\": true}";
    }
};

int main() {
    FileManagerApp app;
    
    if (!app.Initialize()) {
        return -1;
    }
    
    app.CreateWindow({
        .title = "MikoView File Manager",
        .width = 1000,
        .height = 700,
        .url = "file://renderer/index.html"
    });
    
    return app.Run();
}
```

### HTML/JavaScript Code

```html
<!DOCTYPE html>
<html>
<head>
    <title>File Manager</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }
        .toolbar { margin-bottom: 20px; }
        .file-list { border: 1px solid #ccc; height: 500px; overflow-y: auto; }
        .file-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }
        .file-item:hover { background-color: #f5f5f5; }
        .folder { font-weight: bold; color: #0066cc; }
    </style>
</head>
<body>
    <div class="toolbar">
        <input type="text" id="pathInput" placeholder="Enter path..." style="width: 300px;">
        <button id="browseBtn">Browse</button>
        <button id="upBtn">Up</button>
    </div>
    
    <div class="file-list" id="fileList">
        <!-- Files will be listed here -->
    </div>
    
    <script>
        let currentPath = '';
        
        document.getElementById('browseBtn').addEventListener('click', async () => {
            const path = document.getElementById('pathInput').value || '.';
            await browsePath(path);
        });
        
        document.getElementById('upBtn').addEventListener('click', async () => {
            const parentPath = await mikoview.fs.dirname(currentPath);
            await browsePath(parentPath);
        });
        
        async function browsePath(path) {
            try {
                const entries = await mikoview.fs.readDir(path);
                currentPath = path;
                document.getElementById('pathInput').value = path;
                
                const fileList = document.getElementById('fileList');
                fileList.innerHTML = '';
                
                entries.forEach(entry => {
                    const div = document.createElement('div');
                    div.className = 'file-item' + (entry.isDirectory ? ' folder' : '');
                    div.textContent = entry.name;
                    div.addEventListener('click', async () => {
                        if (entry.isDirectory) {
                            await browsePath(entry.path);
                        } else {
                            await openFile(entry.path);
                        }
                    });
                    fileList.appendChild(div);
                });
            } catch (error) {
                alert('Error browsing path: ' + error.message);
            }
        }
        
        async function openFile(path) {
            try {
                const content = await mikoview.fs.readFile(path);
                alert('File content (first 100 chars):\n' + content.substring(0, 100));
            } catch (error) {
                alert('Error opening file: ' + error.message);
            }
        }
        
        // Initialize with current directory
        browsePath('.');
    </script>
</body>
</html>
```

## Text Editor

A simple text editor with save/load functionality.

```html
<!DOCTYPE html>
<html>
<head>
    <title>MikoView Text Editor</title>
    <style>
        body { margin: 0; font-family: Arial, sans-serif; }
        .toolbar { background: #f0f0f0; padding: 10px; border-bottom: 1px solid #ccc; }
        .toolbar button { margin-right: 10px; }
        #editor { width: 100%; height: calc(100vh - 60px); border: none; padding: 10px; font-family: monospace; }
    </style>
</head>
<body>
    <div class="toolbar">
        <button id="newBtn">New</button>
        <button id="openBtn">Open</button>
        <button id="saveBtn">Save</button>
        <button id="saveAsBtn">Save As</button>
        <span id="filename">Untitled</span>
    </div>
    
    <textarea id="editor" placeholder="Start typing..."></textarea>
    
    <script>
        let currentFile = null;
        let isModified = false;
        
        const editor = document.getElementById('editor');
        const filenameEl = document.getElementById('filename');
        
        editor.addEventListener('input', () => {
            isModified = true;
            updateTitle();
        });
        
        document.getElementById('newBtn').addEventListener('click', () => {
            if (isModified && !confirm('Discard changes?')) return;
            editor.value = '';
            currentFile = null;
            isModified = false;
            updateTitle();
        });
        
        document.getElementById('openBtn').addEventListener('click', async () => {
            const path = prompt('Enter file path:');
            if (!path) return;
            
            try {
                const content = await mikoview.fs.readFile(path);
                editor.value = content;
                currentFile = path;
                isModified = false;
                updateTitle();
            } catch (error) {
                alert('Error opening file: ' + error.message);
            }
        });
        
        document.getElementById('saveBtn').addEventListener('click', async () => {
            if (!currentFile) {
                return document.getElementById('saveAsBtn').click();
            }
            
            try {
                await mikoview.fs.writeFile(currentFile, editor.value);
                isModified = false;
                updateTitle();
                alert('File saved!');
            } catch (error) {
                alert('Error saving file: ' + error.message);
            }
        });
        
        document.getElementById('saveAsBtn').addEventListener('click', async () => {
            const path = prompt('Enter file path:');
            if (!path) return;
            
            try {
                await mikoview.fs.writeFile(path, editor.value, { createDirs: true });
                currentFile = path;
                isModified = false;
                updateTitle();
                alert('File saved!');
            } catch (error) {
                alert('Error saving file: ' + error.message);
            }
        });
        
        function updateTitle() {
            const filename = currentFile ? currentFile.split('/').pop() : 'Untitled';
            filenameEl.textContent = filename + (isModified ? ' *' : '');
        }
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            if (e.ctrlKey || e.metaKey) {
                switch (e.key) {
                    case 'n':
                        e.preventDefault();
                        document.getElementById('newBtn').click();
                        break;
                    case 'o':
                        e.preventDefault();
                        document.getElementById('openBtn').click();
                        break;
                    case 's':
                        e.preventDefault();
                        if (e.shiftKey) {
                            document.getElementById('saveAsBtn').click();
                        } else {
                            document.getElementById('saveBtn').click();
                        }
                        break;
                }
            }
        });
    </script>
</body>
</html>
```

## System Information App

Displays system information using custom native handlers.

```cpp
#include "mikoview.hpp"
#include <json/json.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

class SystemInfoApp : public MikoView::Application {
public:
    bool Initialize() override {
        if (!MikoView::Application::Initialize()) {
            return false;
        }
        
        RegisterHandler("system.getInfo", [this](const std::string& data) {
            return GetSystemInfo();
        });
        
        return true;
    }
    
private:
    std::string GetSystemInfo() {
        Json::Value info;
        
#ifdef _WIN32
        info["platform"] = "Windows";
        
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        info["processors"] = sysInfo.dwNumberOfProcessors;
        
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        info["totalMemory"] = static_cast<Json::Int64>(memInfo.ullTotalPhys);
        info["availableMemory"] = static_cast<Json::Int64>(memInfo.ullAvailPhys);
#else
        struct utsname unameData;
        uname(&unameData);
        info["platform"] = unameData.sysname;
        info["version"] = unameData.release;
        info["architecture"] = unameData.machine;
        
        info["processors"] = sysconf(_SC_NPROCESSORS_ONLN);
#endif
        
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, info);
    }
};
```