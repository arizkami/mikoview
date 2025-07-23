# Getting Started with MikoView

## Installation

### From Source

1. **Clone the repository**
   ```bash
   git clone https://github.com/mikoview/mikoview.git
   cd mikoview
   ```

2. **Install dependencies**
   - Download CEF Binary Distribution for your platform
   - Install SDL2 development libraries
   - Install CMake 3.15+

3. **Build the framework**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

## Project Structure

```
my-mikoview-app/
├── src/
│   └── main.cpp          # Application entry point
├── renderer/
│   ├── index.html        # Main HTML file
│   ├── style.css         # Styles
│   └── script.js         # JavaScript logic
├── assets/
│   └── icon.png          # Application icon
└── CMakeLists.txt        # Build configuration
```

## Basic Application

### 1. Main Application (C++)

```cpp
#include "mikoview.hpp"

class MyApp : public MikoView::Application {
public:
    bool Initialize() override {
        if (!MikoView::Application::Initialize()) {
            return false;
        }
        
        // Register custom handlers
        RegisterHandler("app.getVersion", [](const std::string& data) {
            return "{\"version\": \"1.0.0\"}";
        });
        
        return true;
    }
    
    void OnWindowReady() override {
        // Window is ready, you can interact with the renderer
        InvokeRenderer("app.ready", "{}");
    }
};

int main() {
    MyApp app;
    
    if (!app.Initialize()) {
        return -1;
    }
    
    app.CreateWindow({
        .title = "My MikoView App",
        .width = 1200,
        .height = 800,
        .url = "file://renderer/index.html",
        .resizable = true,
        .darkMode = true
    });
    
    return app.Run();
}
```

### 2. Renderer (HTML/JavaScript)

```html
<!DOCTYPE html>
<html>
<head>
    <title>My MikoView App</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div id="app">
        <h1>Welcome to MikoView</h1>
        <button id="getVersion">Get Version</button>
        <p id="version"></p>
    </div>
    
    <script src="script.js"></script>
</body>
</html>
```

```javascript
// script.js
document.addEventListener('DOMContentLoaded', () => {
    const button = document.getElementById('getVersion');
    const versionEl = document.getElementById('version');
    
    button.addEventListener('click', async () => {
        try {
            const result = await mikoview.invoke('app.getVersion');
            const data = JSON.parse(result);
            versionEl.textContent = `Version: ${data.version}`;
        } catch (error) {
            console.error('Error:', error);
        }
    });
    
    // Listen for app ready event
    mikoview.on('app.ready', () => {
        console.log('App is ready!');
    });
});
```

## File System Operations

```javascript
// Reading a file
const content = await mikoview.fs.readFile('path/to/file.txt');

// Writing a file
await mikoview.fs.writeFile('path/to/output.txt', 'Hello, World!');

// Reading directory
const entries = await mikoview.fs.readDir('path/to/directory');

// Check if file exists
const exists = await mikoview.fs.exists('path/to/file.txt');
```

## Next Steps

- [API Reference](api-reference.md) - Complete API documentation
- [Examples](examples.md) - More complex examples
- [Building](building.md) - Advanced build configurations