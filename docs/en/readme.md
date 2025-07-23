# MikoView Framework

MikoView is a modern desktop application framework that combines the power of CEF (Chromium Embedded Framework) with SDL for creating cross-platform desktop applications using web technologies.

## Features

- 🚀 **Modern Web Technologies**: Build desktop apps with HTML, CSS, and JavaScript/TypeScript
- 🎯 **Cross-Platform**: Support for Windows and Linux
- ⚡ **High Performance**: Native performance with CEF and SDL integration
- 🎨 **Native Look & Feel**: Platform-specific theming and dark mode support
- 📁 **File System Access**: Secure file system operations
- 🔧 **TypeScript Support**: Full TypeScript API definitions
- 🎮 **SDL Integration**: Hardware-accelerated rendering and input handling

## Quick Start

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler
- CEF Binary Distribution
- SDL2 development libraries
- Node.js 16+ (for renderer development)

### Building

```bash
# Clone the repository
git clone https://github.com/arizkami/mikoview.git
cd mikoview

# Build the project
mkdir build
cd build
cmake ..
cmake --build .
```

### Your First App

```cpp
#include "mikoview.hpp"

int main() {
    MikoView::Application app;
    
    // Initialize the application
    if (!app.Initialize()) {
        return -1;
    }
    
    // Create a window
    app.CreateWindow({
        .title = "My First MikoView App",
        .width = 1200,
        .height = 800,
        .url = "file://app/index.html"
    });
    
    // Run the application
    return app.Run();
}
```

## Documentation Structure

- [Getting Started](getting-started.md)
- [API Reference](api-reference.md)
- [Examples](examples.md)
- [Building from Source](building.md)
- [Deployment](deployment.md)
- [Contributing](contributing.md)

## License

MIT License - see [LICENSE](../../LICENSE) for details.