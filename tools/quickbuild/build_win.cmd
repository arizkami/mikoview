@echo off
REM MikoView Quick Build Script for Windows
REM This script automates the build process for MikoView on Windows systems

setlocal EnableDelayedExpansion

REM Configuration
set "BUILD_TYPE=Release"
set "BUILD_EXAMPLES=ON"
set "BUILD_TESTS=OFF"
set "BUILD_DOCS=OFF"
set "ENABLE_HOT_RELOAD=ON"
set "DEV_SERVER_URL=http://localhost:3000"
set "DEBUG_PORT=9222"
set "WINDOW_WIDTH=1200"
set "WINDOW_HEIGHT=800"
set "START_HIDDEN=ON"
set "WIN32_CONSOLE=OFF"
set "WIN32_MANIFEST=ON"
set "GENERATOR=Visual Studio 17 2022"
set "PLATFORM=x64"

REM Directories
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\..\."
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "INSTALL_DIR=%PROJECT_ROOT%\install"
set "DIST_DIR=%PROJECT_ROOT%\dist"

REM Colors (if supported)
set "COLOR_INFO=[94m"
set "COLOR_SUCCESS=[92m"
set "COLOR_WARNING=[93m"
set "COLOR_ERROR=[91m"
set "COLOR_RESET=[0m"

REM Flags
set "CREATE_PACKAGE=true"
set "CLEAN_BUILD=false"
set "OPEN_VS=false"

REM Function to print colored output
:print_status
echo %COLOR_INFO%[INFO]%COLOR_RESET% %~1
goto :eof

:print_success
echo %COLOR_SUCCESS%[SUCCESS]%COLOR_RESET% %~1
goto :eof

:print_warning
echo %COLOR_WARNING%[WARNING]%COLOR_RESET% %~1
goto :eof

:print_error
echo %COLOR_ERROR%[ERROR]%COLOR_RESET% %~1
goto :eof

REM Function to check if command exists
:command_exists
where %1 >nul 2>&1
if %errorlevel% equ 0 (
    exit /b 0
) else (
    exit /b 1
)

REM Function to check dependencies
:check_dependencies
call :print_status "Checking dependencies..."

set "missing_deps="

call :command_exists cmake
if %errorlevel% neq 0 (
    set "missing_deps=!missing_deps! cmake"
)

call :command_exists git
if %errorlevel% neq 0 (
    set "missing_deps=!missing_deps! git"
)

REM Check for Visual Studio or Build Tools
set "vs_found=false"
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" set "vs_found=true"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022" set "vs_found=true"
if exist "%ProgramFiles%\Microsoft Visual Studio\2019" (
    set "vs_found=true"
    set "GENERATOR=Visual Studio 16 2019"
)
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
    set "vs_found=true"
    set "GENERATOR=Visual Studio 16 2019"
)

if "%vs_found%"=="false" (
    set "missing_deps=!missing_deps! visual-studio-build-tools"
)

if not "!missing_deps!"=="" (
    call :print_error "Missing dependencies:!missing_deps!"
    call :print_status "Please install:"
    echo   - CMake: https://cmake.org/download/
    echo   - Git: https://git-scm.com/download/win
    echo   - Visual Studio 2019/2022 or Build Tools
    exit /b 1
)

call :print_success "All dependencies are available"
goto :eof

REM Function to setup build directory
:setup_build_dir
call :print_status "Setting up build directory..."

if exist "%BUILD_DIR%" (
    call :print_warning "Build directory exists, cleaning..."
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%" 2>nul
mkdir "%INSTALL_DIR%" 2>nul
mkdir "%DIST_DIR%" 2>nul

call :print_success "Build directory setup complete"
goto :eof

REM Function to configure CMake
:configure_cmake
call :print_status "Configuring CMake..."

cd /d "%BUILD_DIR%"

cmake "%PROJECT_ROOT%" ^
    -G "%GENERATOR%" ^
    -A "%PLATFORM%" ^
    -DCMAKE_BUILD_TYPE="%BUILD_TYPE%" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DMIKO_BUILD_EXAMPLES="%BUILD_EXAMPLES%" ^
    -DMIKO_BUILD_TESTS="%BUILD_TESTS%" ^
    -DMIKO_BUILD_DOCS="%BUILD_DOCS%" ^
    -DMIKO_ENABLE_HOT_RELOAD="%ENABLE_HOT_RELOAD%" ^
    -DMIKO_DEV_SERVER_URL="%DEV_SERVER_URL%" ^
    -DMIKO_DEBUG_PORT="%DEBUG_PORT%" ^
    -DMIKO_DEFAULT_WINDOW_WIDTH="%WINDOW_WIDTH%" ^
    -DMIKO_DEFAULT_WINDOW_HEIGHT="%WINDOW_HEIGHT%" ^
    -DMIKO_START_HIDDEN="%START_HIDDEN%" ^
    -DMIKO_WIN32_CONSOLE="%WIN32_CONSOLE%" ^
    -DMIKO_WIN32_MANIFEST="%WIN32_MANIFEST%"

if %errorlevel% equ 0 (
    call :print_success "CMake configuration complete"
) else (
    call :print_error "CMake configuration failed"
    exit /b 1
)
goto :eof

REM Function to build project
:build_project
call :print_status "Building project..."

cd /d "%BUILD_DIR%"

REM Determine number of CPU cores for parallel build
set "num_cores=%NUMBER_OF_PROCESSORS%"
call :print_status "Building with %num_cores% parallel jobs"

cmake --build . --config "%BUILD_TYPE%" --parallel %num_cores%

if %errorlevel% equ 0 (
    call :print_success "Build complete"
) else (
    call :print_error "Build failed"
    exit /b 1
)
goto :eof

REM Function to install project
:install_project
call :print_status "Installing project..."

cd /d "%BUILD_DIR%"
cmake --install . --config "%BUILD_TYPE%"

if %errorlevel% equ 0 (
    call :print_success "Installation complete"
) else (
    call :print_error "Installation failed"
    exit /b 1
)
goto :eof

REM Function to create distribution package
:create_package
call :print_status "Creating distribution package..."

cd /d "%BUILD_DIR%"

REM Create packages
cmake --build . --config "%BUILD_TYPE%" --target package

REM Move packages to dist directory
if exist "*.zip" (
    move "*.zip" "%DIST_DIR%\" >nul 2>&1
    call :print_success "ZIP package created in %DIST_DIR%"
)

if exist "*.exe" (
    move "*.exe" "%DIST_DIR%\" >nul 2>&1
    call :print_success "NSIS installer created in %DIST_DIR%"
)

if exist "*.msi" (
    move "*.msi" "%DIST_DIR%\" >nul 2>&1
    call :print_success "MSI installer created in %DIST_DIR%"
)
goto :eof

REM Function to run tests
:run_tests
if "%BUILD_TESTS%"=="ON" (
    call :print_status "Running tests..."
    cd /d "%BUILD_DIR%"
    ctest --output-on-failure -C "%BUILD_TYPE%"
    
    if %errorlevel% equ 0 (
        call :print_success "All tests passed"
    ) else (
        call :print_error "Some tests failed"
        exit /b 1
    )
)
goto :eof

REM Function to open Visual Studio
:open_visual_studio
if "%OPEN_VS%"=="true" (
    call :print_status "Opening Visual Studio..."
    cd /d "%BUILD_DIR%"
    if exist "MikoView.sln" (
        start "" "MikoView.sln"
        call :print_success "Visual Studio opened"
    ) else (
        call :print_warning "Solution file not found"
    )
)
goto :eof

REM Function to show usage
:show_usage
echo Usage: %~nx0 [OPTIONS]
echo Options:
echo   --debug           Build in Debug mode
echo   --release         Build in Release mode (default)
echo   --no-examples     Don't build examples
echo   --with-tests      Build and run tests
echo   --with-docs       Build documentation
echo   --no-package      Skip package creation
echo   --clean           Clean build directory before building
echo   --open-vs         Open Visual Studio after configuration
echo   --console         Show console window in built application
echo   --vs2019          Use Visual Studio 2019 generator
echo   --vs2022          Use Visual Studio 2022 generator (default)
echo   --x86             Build for x86 platform
echo   --x64             Build for x64 platform (default)
echo   --help            Show this help message
echo.
echo Environment variables:
echo   DEV_SERVER_URL    Development server URL (default: http://localhost:3000)
echo   DEBUG_PORT        Remote debugging port (default: 9222)
echo   WINDOW_WIDTH      Default window width (default: 1200)
echo   WINDOW_HEIGHT     Default window height (default: 800)
goto :eof

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done

if "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if "%~1"=="--release" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if "%~1"=="--no-examples" (
    set "BUILD_EXAMPLES=OFF"
    shift
    goto :parse_args
)
if "%~1"=="--with-tests" (
    set "BUILD_TESTS=ON"
    shift
    goto :parse_args
)
if "%~1"=="--with-docs" (
    set "BUILD_DOCS=ON"
    shift
    goto :parse_args
)
if "%~1"=="--no-package" (
    set "CREATE_PACKAGE=false"
    shift
    goto :parse_args
)
if "%~1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if "%~1"=="--open-vs" (
    set "OPEN_VS=true"
    shift
    goto :parse_args
)
if "%~1"=="--console" (
    set "WIN32_CONSOLE=ON"
    shift
    goto :parse_args
)
if "%~1"=="--vs2019" (
    set "GENERATOR=Visual Studio 16 2019"
    shift
    goto :parse_args
)
if "%~1"=="--vs2022" (
    set "GENERATOR=Visual Studio 17 2022"
    shift
    goto :parse_args
)
if "%~1"=="--x86" (
    set "PLATFORM=Win32"
    shift
    goto :parse_args
)
if "%~1"=="--x64" (
    set "PLATFORM=x64"
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    call :show_usage
    exit /b 0
)

call :print_error "Unknown option: %~1"
call :show_usage
exit /b 1

:args_done

REM Override with environment variables if set
if not "%DEV_SERVER_URL%"=="" set "DEV_SERVER_URL=%DEV_SERVER_URL%"
if not "%DEBUG_PORT%"=="" set "DEBUG_PORT=%DEBUG_PORT%"
if not "%WINDOW_WIDTH%"=="" set "WINDOW_WIDTH=%WINDOW_WIDTH%"
if not "%WINDOW_HEIGHT%"=="" set "WINDOW_HEIGHT=%WINDOW_HEIGHT%"

REM Main execution
call :parse_args %*

call :print_status "Starting MikoView build process..."
call :print_status "Build Type: %BUILD_TYPE%"
call :print_status "Generator: %GENERATOR%"
call :print_status "Platform: %PLATFORM%"
call :print_status "Project Root: %PROJECT_ROOT%"
call :print_status "Build Directory: %BUILD_DIR%"

REM Execute build steps
call :check_dependencies
if %errorlevel% neq 0 exit /b 1

if "%CLEAN_BUILD%"=="true" (
    call :setup_build_dir
    if %errorlevel% neq 0 exit /b 1
)

if not exist "%BUILD_DIR%" (
    call :setup_build_dir
    if %errorlevel% neq 0 exit /b 1
)

call :configure_cmake
if %errorlevel% neq 0 exit /b 1

call :build_project
if %errorlevel% neq 0 exit /b 1

call :run_tests
if %errorlevel% neq 0 exit /b 1

call :install_project
if %errorlevel% neq 0 exit /b 1

if "%CREATE_PACKAGE%"=="true" (
    call :create_package
    if %errorlevel% neq 0 exit /b 1
)

call :open_visual_studio

call :print_success "MikoView build process completed successfully!"
call :print_status "Executable location: %INSTALL_DIR%\bin\MikoView.exe"
if "%CREATE_PACKAGE%"=="true" (
    call :print_status "Packages location: %DIST_DIR%"
)

echo.
echo Press any key to exit...
pause >nul