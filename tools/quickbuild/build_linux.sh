#!/bin/bash

# MikoView Quick Build Script for Linux
# This script automates the build process for MikoView on Linux systems

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Release"
BUILD_EXAMPLES="ON"
BUILD_TESTS="OFF"
BUILD_DOCS="OFF"
ENABLE_HOT_RELOAD="ON"
DEV_SERVER_URL="http://localhost:3000"
DEBUG_PORT="9222"
WINDOW_WIDTH="1200"
WINDOW_HEIGHT="800"
START_HIDDEN="ON"

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$PROJECT_ROOT/install"
DIST_DIR="$PROJECT_ROOT/dist"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists pkg-config; then
        missing_deps+=("pkg-config")
    fi
    
    if ! command_exists git; then
        missing_deps+=("git")
    fi
    
    # Check for required libraries
    if ! pkg-config --exists x11; then
        missing_deps+=("libx11-dev")
    fi
    
    if ! pkg-config --exists xrandr; then
        missing_deps+=("libxrandr-dev")
    fi
    
    if ! pkg-config --exists xi; then
        missing_deps+=("libxi-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install them using:"
        echo "sudo apt-get update"
        echo "sudo apt-get install ${missing_deps[*]}"
        exit 1
    fi
    
    print_success "All dependencies are available"
}

# Function to setup build directory
setup_build_dir() {
    print_status "Setting up build directory..."
    
    if [ -d "$BUILD_DIR" ]; then
        print_warning "Build directory exists, cleaning..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    mkdir -p "$INSTALL_DIR"
    mkdir -p "$DIST_DIR"
    
    print_success "Build directory setup complete"
}

# Function to configure CMake
configure_cmake() {
    print_status "Configuring CMake..."
    
    cd "$BUILD_DIR"
    
    cmake "$PROJECT_ROOT" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        -DMIKO_BUILD_EXAMPLES="$BUILD_EXAMPLES" \
        -DMIKO_BUILD_TESTS="$BUILD_TESTS" \
        -DMIKO_BUILD_DOCS="$BUILD_DOCS" \
        -DMIKO_ENABLE_HOT_RELOAD="$ENABLE_HOT_RELOAD" \
        -DMIKO_DEV_SERVER_URL="$DEV_SERVER_URL" \
        -DMIKO_DEBUG_PORT="$DEBUG_PORT" \
        -DMIKO_DEFAULT_WINDOW_WIDTH="$WINDOW_WIDTH" \
        -DMIKO_DEFAULT_WINDOW_HEIGHT="$WINDOW_HEIGHT" \
        -DMIKO_START_HIDDEN="$START_HIDDEN" \
        -DMIKO_LINUX_X11=ON \
        -DMIKO_LINUX_WAYLAND=OFF
    
    if [ $? -eq 0 ]; then
        print_success "CMake configuration complete"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
}

# Function to build project
build_project() {
    print_status "Building project..."
    
    cd "$BUILD_DIR"
    
    # Determine number of CPU cores for parallel build
    local num_cores=$(nproc)
    print_status "Building with $num_cores parallel jobs"
    
    make -j"$num_cores"
    
    if [ $? -eq 0 ]; then
        print_success "Build complete"
    else
        print_error "Build failed"
        exit 1
    fi
}

# Function to install project
install_project() {
    print_status "Installing project..."
    
    cd "$BUILD_DIR"
    make install
    
    if [ $? -eq 0 ]; then
        print_success "Installation complete"
    else
        print_error "Installation failed"
        exit 1
    fi
}

# Function to create distribution package
create_package() {
    print_status "Creating distribution package..."
    
    cd "$BUILD_DIR"
    
    # Create tarball
    make package
    
    # Move packages to dist directory
    if [ -f *.tar.gz ]; then
        mv *.tar.gz "$DIST_DIR/"
        print_success "Tarball package created in $DIST_DIR"
    fi
    
    if [ -f *.deb ]; then
        mv *.deb "$DIST_DIR/"
        print_success "DEB package created in $DIST_DIR"
    fi
    
    if [ -f *.rpm ]; then
        mv *.rpm "$DIST_DIR/"
        print_success "RPM package created in $DIST_DIR"
    fi
}

# Function to run tests
run_tests() {
    if [ "$BUILD_TESTS" = "ON" ]; then
        print_status "Running tests..."
        cd "$BUILD_DIR"
        ctest --output-on-failure
        
        if [ $? -eq 0 ]; then
            print_success "All tests passed"
        else
            print_error "Some tests failed"
            exit 1
        fi
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --debug           Build in Debug mode"
    echo "  --release         Build in Release mode (default)"
    echo "  --no-examples     Don't build examples"
    echo "  --with-tests      Build and run tests"
    echo "  --with-docs       Build documentation"
    echo "  --no-package      Skip package creation"
    echo "  --clean           Clean build directory before building"
    echo "  --help            Show this help message"
    echo ""
    echo "Environment variables:"
    echo "  DEV_SERVER_URL    Development server URL (default: http://localhost:3000)"
    echo "  DEBUG_PORT        Remote debugging port (default: 9222)"
    echo "  WINDOW_WIDTH      Default window width (default: 1200)"
    echo "  WINDOW_HEIGHT     Default window height (default: 800)"
}

# Parse command line arguments
CREATE_PACKAGE="true"
CLEAN_BUILD="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES="OFF"
            shift
            ;;
        --with-tests)
            BUILD_TESTS="ON"
            shift
            ;;
        --with-docs)
            BUILD_DOCS="ON"
            shift
            ;;
        --no-package)
            CREATE_PACKAGE="false"
            shift
            ;;
        --clean)
            CLEAN_BUILD="true"
            shift
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Override with environment variables if set
[ -n "$DEV_SERVER_URL" ] && DEV_SERVER_URL="$DEV_SERVER_URL"
[ -n "$DEBUG_PORT" ] && DEBUG_PORT="$DEBUG_PORT"
[ -n "$WINDOW_WIDTH" ] && WINDOW_WIDTH="$WINDOW_WIDTH"
[ -n "$WINDOW_HEIGHT" ] && WINDOW_HEIGHT="$WINDOW_HEIGHT"

# Main execution
print_status "Starting MikoView build process..."
print_status "Build Type: $BUILD_TYPE"
print_status "Project Root: $PROJECT_ROOT"
print_status "Build Directory: $BUILD_DIR"

# Execute build steps
check_dependencies

if [ "$CLEAN_BUILD" = "true" ]; then
    setup_build_dir
fi

if [ ! -d "$BUILD_DIR" ]; then
    setup_build_dir
fi

configure_cmake
build_project
run_tests
install_project

if [ "$CREATE_PACKAGE" = "true" ]; then
    create_package
fi

print_success "MikoView build process completed successfully!"
print_status "Executable location: $INSTALL_DIR/bin/MikoView"
if [ "$CREATE_PACKAGE" = "true" ]; then
    print_status "Packages location: $DIST_DIR"
fi