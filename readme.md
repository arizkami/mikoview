# MikoView

![Development Status](https://img.shields.io/badge/status-in%20development-yellow)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)
![Framework](https://img.shields.io/badge/framework-CEF%20%7C%20SDL2-green)
![Frontend](https://img.shields.io/badge/frontend-React%20%7C%20Vue%20%7C%20TypeScript-61DAFB)
![License](https://img.shields.io/badge/license-MIT-green)
![Build System](https://img.shields.io/badge/build-CMake-red)
![Version](https://img.shields.io/badge/version-0.1.0--dev-orange)

A modern, production-ready desktop application framework built with React/Vue, CEF (Chromium Embedded Framework), and SDL2. MikoView provides an Electron-like development experience with native performance and cross-platform compatibility.

## ✨ Features

### 🚀 **Core Framework**
- **Modern C++17** architecture with clean API design
- **Production-ready** framework with modular CMake build system
- **Cross-platform** support (Windows, Linux, macOS)
- **Electron-style** window behavior (hidden until content loads)
- **Dark mode** support with native OS integration
- **Hot reload** development server integration

### 🎨 **Frontend Flexibility**
- **React** with TypeScript support
- **Vue.js** with modern tooling
- **Bun** runtime for fast development
- **API testing** capabilities built-in
- **Asset bundling** and optimization

### 🔧 **Developer Experience**
- **Modular CMake** system with automatic dependency management
- **Automatic CEF/SDL2** download and configuration
- **Remote debugging** support (Chrome DevTools)
- **Comprehensive logging** system
- **JavaScript API** for native integration
- **Production packaging** (NSIS for Windows, DEB for Linux)

## 📋 Prerequisites

### Windows
- **Visual Studio 2019** or later with C++ development tools
- **CMake** 3.19 or higher
- **Git** for dependency management
- **NSIS** (optional, for creating installers)

### Linux
- **GCC 9+** or **Clang 10+** with C++17 support
- **CMake** 3.19 or higher
- **Git** for dependency management
- **X11 development libraries**
- **dpkg-dev** (optional, for creating .deb packages)

### Development (Optional)
- **Bun** or **Node.js** for frontend development
- **TypeScript** for enhanced development experience

## 🚀 Quick Start

### 1. Clone and Build

```bash
# Clone the repository
git clone https://github.com/arizkami/mikoview
cd mikoview

# Configure and build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run the application
.\build\Release\MikoView.exe  # Windows
./build/MikoView                # Linux
```

### 2. Development Mode

```bash
# Start frontend development server
cd renderer/react  # or renderer/vue
bun install
bun run dev

# Build in debug mode (connects to dev server)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
.\build\Debug\MikoView.exe
```

### 3. Create Production Installer

```bash
# Generate installer packages
python tools/makeinstaller.py
```

## 🏗️ Project Architecture

```
mikoview/
├── 📁 cmake/                   # Modular CMake system
│   ├── MikoConfig.cmake        # Project configuration
│   ├── MikoDependencies.cmake  # External dependencies
│   ├── MikoMacros.cmake        # Reusable functions
│   └── MikoPlatform.cmake      # Platform detection
├── 📁 mikoview/                # Core framework
│   ├── 📁 gui/                 # Platform-specific GUI
│   ├── 📁 jsapi/               # JavaScript API bridge
│   ├── mikoapp.hpp/cpp         # CEF application
│   ├── mikoclient.hpp/cpp      # CEF browser client
│   ├── logger.hpp/cpp          # Logging system
│   └── app_config.hpp/cpp      # Configuration management
├── 📁 example/                 # Example application
│   ├── main.cpp                # Application entry point
│   └── 📁 assets/              # Application resources
├── 📁 renderer/                # Frontend frameworks
│   ├── 📁 react/               # React + TypeScript
│   ├── 📁 vue/                 # Vue.js + TypeScript
│   └── 📁 api/                 # API testing tools
├── 📁 tools/                   # Build and packaging tools
│   ├── makeinstaller.py        # Installer generator
│   └── iconconvert.py          # Icon conversion utility
├── 📁 docs/                    # Documentation
├── mikoview.hpp                # Main framework header
├── mikoview.cpp                # Framework implementation
└── CMakeLists.txt              # Main build configuration
```

## 🔧 Framework Usage

### Simple Application

```cpp
#include "mikoview.hpp"

int main() {
    // One-liner application
    return MikoView::Quick::RunApp(
        "http://localhost:3000",  // URL
        "My Application",         // Title
        true                       // Debug mode
    );
}
```

### Advanced Configuration

```cpp
#include "mikoview.hpp"

int main() {
    MikoView::Config config;
    config.window_title = "Production App";
    config.window_width = 1400;
    config.window_height = 900;
    config.debug_mode = false;
    config.startup_url = "file:///app/index.html";
    config.start_hidden = true;  // Electron-style
    
    auto app = MikoView::Quick::CreateApp(config);
    if (!app) {
        return -1;
    }
    
    // Set callbacks
    app->SetReadyCallback([]() {
        MikoView::Utils::LogInfo("Application ready!");
    });
    
    return app->Run();
}
```

## 🎨 Frontend Development

### React Development

```bash
cd renderer/react
bun install
bun run dev      # Development server
bun run build    # Production build
bun run preview  # Preview production build
```

### Vue Development

```bash
cd renderer/vue
npm install
npm run dev      # Development server
npm run build    # Production build
npm run preview  # Preview production build
```

### JavaScript API Integration

```javascript
// Call native functions from JavaScript
window.mikoview.invoke('showNotification', {
    title: 'Hello',
    message: 'From native code!'
});

// File system access
const files = await window.mikoview.invoke('readDirectory', '/path/to/dir');
```

## 🔧 Build Configuration

### CMake Options

```bash
# Enable/disable features
cmake -B build -S . \
  -DMIKO_BUILD_EXAMPLES=ON \
  -DMIKO_BUILD_TESTS=OFF \
  -DMIKO_ENABLE_LOGGING=ON \
  -DCMAKE_BUILD_TYPE=Release
```

### Platform-Specific Options

```bash
# Windows with console (debug)
cmake -B build -S . -DMIKO_WIN32_CONSOLE=ON

# Linux with specific CEF version
cmake -B build -S . -DCEF_VERSION="120.1.10+g3ce3184+chromium-120.0.6099.129"
```

## 📦 Packaging and Distribution

### Windows Installer (NSIS)

```bash
python tools/makeinstaller.py --platform windows
# Generates: MikoView-Setup.exe
```

### Linux Package (DEB)

```bash
python tools/makeinstaller.py --platform linux
# Generates: mikoview_0.1.0_amd64.deb
```

### Manual Packaging

```bash
# Using CPack
cd build
cpack -G NSIS     # Windows
cpack -G DEB      # Linux
```

## 🐛 Troubleshooting

### Build Issues

| Issue | Solution |
|-------|----------|
| **CMake not found** | Install CMake 3.19+ and add to PATH |
| **CEF download fails** | Check internet connection and firewall |
| **Visual Studio errors** | Install C++ development workload |
| **Linux build fails** | Install X11 development libraries |

### Runtime Issues

| Issue | Solution |
|-------|----------|
| **Application won't start** | Check `mikoview.log` for errors |
| **Dev server connection fails** | Ensure React/Vue dev server is running |
| **Window not showing** | Check timeout settings in debug mode |
| **JavaScript API not working** | Verify invoke handlers are registered |

### Debug Mode

```bash
# Enable detailed logging
export MIKO_LOG_LEVEL=DEBUG  # Linux
set MIKO_LOG_LEVEL=DEBUG     # Windows

# Remote debugging (Chrome DevTools)
# Open: http://localhost:9222
```

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Guidelines

- Follow **C++17** standards and best practices
- Use **modern CMake** patterns (3.19+)
- Maintain **cross-platform** compatibility
- Add **comprehensive tests** for new features
- Update **documentation** for API changes

## 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **CEF Team** for the Chromium Embedded Framework
- **SDL Team** for cross-platform window management
- **React/Vue Teams** for excellent frontend frameworks
- **CMake Community** for build system excellence

---

**MikoView** - Building the future of desktop applications with web technologies 🚀
