# MikoView

A modern desktop WebView built with React, CEF (Chromium Embedded Framework), and SDL2.

## Features

- Modern React-based user interface
- Native desktop application using CEF
- Cross-platform window management with SDL2
- Hot reload development server
- Integrated API testing capabilities

## Prerequisites

- **Windows**: Visual Studio 2019 or later with C++ development tools
- **CMake**: Version 3.20 or higher
- **Git**: For cloning dependencies
- **Node.js/Bun**: For React development (optional, for frontend development)

## Building

### 1. Clone the Repository

```bash
git clone https://github.com/arizkami/mikoview
cd mikoview
```

### 2. Build the Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release
```

### 3. Development Build (Debug)

```bash
# For debug build with console output
cmake --build . --config Debug
```

## Usage

### Running the Application

After building, run the executable from the build directory:

```bash
# Release build
.\Release\mikoview.exe

# Debug build
.\Debug\mikoview.exe
```

### Development Mode

In debug mode, the application will attempt to connect to a React development server at `http://localhost:3000`. To start the development server:

```bash
cd renderer
bun install
bun run dev
```

### Production Mode

In release mode, the application loads the built React assets from the local `renderer/dist` directory.

## Project Structure

```
mikoview/
├── src/                    # C++ source files
│   ├── main.cpp           # Application entry point
│   ├── simple_app.*       # CEF application setup
│   ├── simple_client.*    # CEF browser client
│   └── logger.*           # Logging utilities
├── renderer/              # React frontend
│   ├── src/               # React source files
│   ├── package.json       # Node.js dependencies
│   └── dist/              # Built assets (generated)
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## Development

### Frontend Development

The frontend is built with React and TypeScript. To work on the frontend:

```bash
cd renderer
bun install
bun run dev
```

This starts a development server at `http://localhost:3000` that the debug build will automatically connect to.

### Backend Development

The C++ backend handles:
- Window management (SDL2)
- Browser embedding (CEF)
- Application lifecycle
- Logging

Modify the source files in the `src/` directory and rebuild using CMake.

### Logging

The application logs to `mikoview.log` in the executable directory. Check this file for debugging information.

## Dependencies

The project automatically downloads and builds the following dependencies:

- **CEF (Chromium Embedded Framework)**: For embedding the web browser
- **SDL2**: For cross-platform window management
- **React**: Frontend framework (managed separately in renderer/)

## Troubleshooting

### Build Issues

1. **CMake not found**: Ensure CMake is installed and in your PATH
2. **Visual Studio errors**: Make sure you have the C++ development workload installed
3. **CEF download fails**: Check your internet connection and firewall settings

### Runtime Issues

1. **Application won't start**: Check `mikoview.log` for error messages
2. **React dev server connection fails**: Ensure the dev server is running on port 3000
3. **Window not responding**: Try rebuilding in debug mode for more detailed logging

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
