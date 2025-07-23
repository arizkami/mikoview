# API Reference

## MikoView::Application

The main application class that manages the application lifecycle.

### Methods

#### `bool Initialize()`
Initializes the application framework.

**Returns:** `true` if initialization was successful, `false` otherwise.

#### `int Run()`
Starts the application main loop.

**Returns:** Exit code (0 for success).

#### `void Shutdown()`
Shuts down the application and cleans up resources.

#### `bool CreateWindow(const WindowConfig& config)`
Creates a new application window.

**Parameters:**
- `config`: Window configuration object

**Returns:** `true` if window creation was successful.

### WindowConfig Structure

```cpp
struct WindowConfig {
    std::string title = "MikoView App";
    int width = 1200;
    int height = 800;
    std::string url;
    bool resizable = true;
    bool darkMode = false;
    bool fullscreen = false;
    bool maximized = false;
    bool minimized = false;
};
```

### Event Handlers

#### `virtual void OnWindowReady()`
Called when the window is ready and the renderer has loaded.

#### `virtual void OnWindowClose()`
Called when the window is about to close.

#### `virtual void OnWindowResize(int width, int height)`
Called when the window is resized.

## JavaScript API

### mikoview.invoke(method, data)

Invokes a native method from the renderer.

**Parameters:**
- `method` (string): The method name to invoke
- `data` (any): Data to pass to the method

**Returns:** Promise that resolves with the result

```javascript
const result = await mikoview.invoke('app.getVersion', {});
```

### mikoview.on(event, callback)

Registers an event listener for native events.

**Parameters:**
- `event` (string): Event name
- `callback` (function): Event handler function

```javascript
mikoview.on('app.ready', () => {
    console.log('App is ready!');
});
```

## File System API

### mikoview.fs.readFile(path, options)

Reads a file from the file system.

**Parameters:**
- `path` (string): File path
- `options` (object): Read options
  - `encoding` (string): 'utf8', 'binary', or 'base64'

**Returns:** Promise that resolves with file content

### mikoview.fs.writeFile(path, data, options)

Writes data to a file.

**Parameters:**
- `path` (string): File path
- `data` (string): Data to write
- `options` (object): Write options
  - `encoding` (string): 'utf8', 'binary', or 'base64'
  - `createDirs` (boolean): Create parent directories if they don't exist

**Returns:** Promise that resolves with bytes written

### mikoview.fs.readDir(path, options)

Reads directory contents.

**Parameters:**
- `path` (string): Directory path
- `options` (object): Read options
  - `recursive` (boolean): Read subdirectories recursively

**Returns:** Promise that resolves with array of directory entries

### mikoview.fs.exists(path)

Checks if a path exists.

**Parameters:**
- `path` (string): Path to check

**Returns:** Promise that resolves with boolean

### mikoview.fs.createDir(path, recursive)

Creates a directory.

**Parameters:**
- `path` (string): Directory path
- `recursive` (boolean): Create parent directories

**Returns:** Promise that resolves when complete

### mikoview.fs.deleteFile(path)

Deletes a file.

**Parameters:**
- `path` (string): File path

**Returns:** Promise that resolves when complete

### mikoview.fs.copyFile(source, destination)

Copies a file.

**Parameters:**
- `source` (string): Source file path
- `destination` (string): Destination file path

**Returns:** Promise that resolves when complete

### mikoview.fs.moveFile(source, destination)

Moves/renames a file.

**Parameters:**
- `source` (string): Source file path
- `destination` (string): Destination file path

**Returns:** Promise that resolves when complete

## Path Utilities

### mikoview.fs.basename(path, ext)

Gets the basename of a path.

### mikoview.fs.dirname(path)

Gets the directory name of a path.

### mikoview.fs.extname(path)

Gets the extension of a path.

### mikoview.fs.joinPath(...segments)

Joins path segments.

### mikoview.fs.resolvePath(path)

Resolves a path to an absolute path.