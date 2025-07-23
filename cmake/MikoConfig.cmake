# MikoConfig.cmake - Project Configuration
# This file contains project-wide configuration settings

# =============================================================================
# Project Version Information
# =============================================================================
set(MIKO_VERSION_MAJOR 1)
set(MIKO_VERSION_MINOR 0)
set(MIKO_VERSION_PATCH 0)
set(MIKO_VERSION_TWEAK 0)

# Construct full version string
set(MIKO_VERSION "${MIKO_VERSION_MAJOR}.${MIKO_VERSION_MINOR}.${MIKO_VERSION_PATCH}")
if(MIKO_VERSION_TWEAK GREATER 0)
    set(MIKO_VERSION "${MIKO_VERSION}.${MIKO_VERSION_TWEAK}")
endif()

message(STATUS "MikoView Version: ${MIKO_VERSION}")

# =============================================================================
# Project Configuration Options
# =============================================================================

# Build options
option(MIKO_BUILD_EXAMPLES "Build example applications" ON)
option(MIKO_BUILD_TESTS "Build test suite" OFF)
option(MIKO_BUILD_DOCS "Build documentation" OFF)
option(MIKO_ENABLE_LOGGING "Enable logging system" ON)
option(MIKO_ENABLE_DEBUG_FEATURES "Enable debug features" OFF)

# Framework options
option(MIKO_STATIC_FRAMEWORK "Build framework as static library" ON)
option(MIKO_SHARED_FRAMEWORK "Build framework as shared library" OFF)

# Platform-specific options
if(WIN32)
    option(MIKO_WIN32_CONSOLE "Show console window on Windows" OFF)
    option(MIKO_WIN32_MANIFEST "Include Windows manifest" ON)
endif()

if(UNIX AND NOT APPLE)
    option(MIKO_LINUX_WAYLAND "Enable Wayland support" OFF)
    option(MIKO_LINUX_X11 "Enable X11 support" ON)
endif()

# =============================================================================
# Compiler Configuration
# =============================================================================

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags
if(MSVC)
    # MSVC-specific flags
    add_compile_options(/W4)  # Warning level 4
    add_compile_options(/permissive-)  # Disable non-conforming code
    
    # Disable specific warnings
    add_compile_options(/wd4251)  # DLL interface warnings
    add_compile_options(/wd4275)  # DLL base class warnings
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Debug flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
    
    # Release flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# =============================================================================
# Build Configuration
# =============================================================================

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =============================================================================
# Feature Configuration
# =============================================================================

# Configure logging
if(MIKO_ENABLE_LOGGING)
    add_compile_definitions(MIKO_ENABLE_LOGGING=1)
else()
    add_compile_definitions(MIKO_ENABLE_LOGGING=0)
endif()

# Configure debug features
if(MIKO_ENABLE_DEBUG_FEATURES OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MIKO_DEBUG=1)
else()
    add_compile_definitions(MIKO_DEBUG=0)
endif()

# =============================================================================
# Installation Configuration
# =============================================================================

# Installation directories
include(GNUInstallDirs)

set(MIKO_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(MIKO_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(MIKO_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MIKO_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}/mikoview)
set(MIKO_INSTALL_DOCDIR ${CMAKE_INSTALL_DOCDIR})

# =============================================================================
# Package Configuration
# =============================================================================

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "MikoView")
set(CPACK_PACKAGE_VENDOR "MikoView Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform web view framework")
set(CPACK_PACKAGE_VERSION ${MIKO_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${MIKO_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MIKO_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MIKO_VERSION_PATCH})

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "MikoView Framework")
    set(CPACK_NSIS_PACKAGE_NAME "MikoView")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MikoView Project")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

# =============================================================================
# Development Configuration
# =============================================================================

# Enable testing if requested
if(MIKO_BUILD_TESTS)
    enable_testing()
    include(CTest)
endif()

# Set startup project for Visual Studio
if(WIN32 AND MIKO_BUILD_EXAMPLES)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

# Print configuration summary
function(print_configuration_summary)
    message(STATUS "=== Configuration Summary ===")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "Examples: ${MIKO_BUILD_EXAMPLES}")
    message(STATUS "Tests: ${MIKO_BUILD_TESTS}")
    message(STATUS "Documentation: ${MIKO_BUILD_DOCS}")
    message(STATUS "Logging: ${MIKO_ENABLE_LOGGING}")
    message(STATUS "Debug Features: ${MIKO_ENABLE_DEBUG_FEATURES}")
    message(STATUS "Static Framework: ${MIKO_STATIC_FRAMEWORK}")
    message(STATUS "Shared Framework: ${MIKO_SHARED_FRAMEWORK}")
    message(STATUS "=============================")
endfunction()