# MikoDependencies.cmake - External Dependencies Management
# This file handles all external dependencies for the MikoView project

# Include FetchContent for downloading dependencies
include(FetchContent)

# =============================================================================
# SDL2 Configuration
# =============================================================================
function(setup_sdl2)
    message(STATUS "Setting up SDL2...")
    
    # Download and build SDL2
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.28.5
        GIT_SHALLOW TRUE
    )
    
    # Configure SDL2 options
    set(SDL_SHARED ON CACHE BOOL "Build SDL2 as shared library")
    set(SDL_STATIC OFF CACHE BOOL "Build SDL2 as static library")
    set(SDL_TEST OFF CACHE BOOL "Build SDL2 test programs")
    
    FetchContent_MakeAvailable(SDL2)
    
    message(STATUS "SDL2 setup complete")
endfunction()

# =============================================================================
# CEF (Chromium Embedded Framework) Configuration
# =============================================================================
function(setup_cef)
    message(STATUS "Setting up CEF...")
    
    # Set CEF version
    set(CEF_VERSION "138.0.27+g0b28f18+chromium-138.0.7204.158" PARENT_SCOPE)
    
    # Determine platform
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(CEF_PLATFORM "windows64" PARENT_SCOPE)
        else()
            set(CEF_PLATFORM "windows32" PARENT_SCOPE)
        endif()
    elseif(APPLE)
        set(CEF_PLATFORM "macosx64" PARENT_SCOPE)
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(CEF_PLATFORM "linux64" PARENT_SCOPE)
        else()
            set(CEF_PLATFORM "linux32" PARENT_SCOPE)
        endif()
    endif()
    
    # Set CEF paths
    set(CEF_DOWNLOAD_URL "https://cef-builds.spotifycdn.com/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}.tar.bz2" PARENT_SCOPE)
    set(CEF_ROOT "${CMAKE_BINARY_DIR}/cef" PARENT_SCOPE)
    set(CEF_ARCHIVE "${CMAKE_BINARY_DIR}/cef_binary.tar.bz2" PARENT_SCOPE)
    set(CEF_EXTRACTED_DIR "${CMAKE_BINARY_DIR}/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}" PARENT_SCOPE)
    
    message(STATUS "CEF configuration complete")
endfunction()

# Function to download and extract CEF
function(download_cef)
    if(NOT EXISTS "${CEF_ROOT}")
        message(STATUS "Downloading CEF from ${CEF_DOWNLOAD_URL}")
        
        # Download CEF
        file(DOWNLOAD "${CEF_DOWNLOAD_URL}" "${CEF_ARCHIVE}"
             SHOW_PROGRESS
             STATUS download_status
             LOG download_log)
        
        list(GET download_status 0 status_code)
        if(NOT status_code EQUAL 0)
            message(FATAL_ERROR "Failed to download CEF: ${download_log}")
        endif()
        
        # Extract CEF
        message(STATUS "Extracting CEF...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xjf "${CEF_ARCHIVE}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            RESULT_VARIABLE extract_result
        )
        
        if(NOT extract_result EQUAL 0)
            message(FATAL_ERROR "Failed to extract CEF archive")
        endif()
        
        # Move extracted directory to CEF_ROOT
        file(RENAME "${CEF_EXTRACTED_DIR}" "${CEF_ROOT}")
        
        # Clean up archive
        file(REMOVE "${CEF_ARCHIVE}")
        
        message(STATUS "CEF downloaded and extracted successfully")
    else()
        message(STATUS "CEF already exists at ${CEF_ROOT}")
    endif()
endfunction()

# Function to configure CEF libraries
function(configure_cef_libraries)
    # Add CEF cmake modules to path
    list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake" PARENT_SCOPE)
    
    # Find CEF
    find_package(CEF REQUIRED)
    
    # Platform-specific CEF library setup
    if(OS_WINDOWS)
        # Create libcef_lib target
        add_library(libcef_lib SHARED IMPORTED)
        set_target_properties(libcef_lib PROPERTIES
            IMPORTED_LOCATION "${CEF_ROOT}/Release/libcef.dll"
            IMPORTED_LOCATION_DEBUG "${CEF_ROOT}/Debug/libcef.dll"
            IMPORTED_LOCATION_RELEASE "${CEF_ROOT}/Release/libcef.dll"
            IMPORTED_IMPLIB "${CEF_ROOT}/Release/libcef.lib"
            IMPORTED_IMPLIB_DEBUG "${CEF_ROOT}/Debug/libcef.lib"
            IMPORTED_IMPLIB_RELEASE "${CEF_ROOT}/Release/libcef.lib"
        )
        
        # Create libcef_dll_wrapper target
        add_subdirectory("${CEF_ROOT}/libcef_dll" libcef_dll_wrapper)
    elseif(OS_LINUX)
        # Create libcef_lib target for Linux
        add_library(libcef_lib SHARED IMPORTED)
        set_target_properties(libcef_lib PROPERTIES
            IMPORTED_LOCATION "${CEF_ROOT}/Release/libcef.so"
            IMPORTED_LOCATION_DEBUG "${CEF_ROOT}/Debug/libcef.so"
            IMPORTED_LOCATION_RELEASE "${CEF_ROOT}/Release/libcef.so"
        )
        
        # Create libcef_dll_wrapper target
        add_subdirectory("${CEF_ROOT}/libcef_dll" libcef_dll_wrapper)
    endif()
endfunction()

# =============================================================================
# Platform-specific Dependencies
# =============================================================================
function(setup_platform_dependencies)
    if(UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(X11 REQUIRED x11)
        set(X11_INCLUDE_DIRS ${X11_INCLUDE_DIRS} PARENT_SCOPE)
        set(X11_LIBRARIES ${X11_LIBRARIES} PARENT_SCOPE)
    endif()
endfunction()

# =============================================================================
# Main Setup Function
# =============================================================================
function(setup_all_dependencies)
    message(STATUS "Setting up all dependencies...")
    
    # Setup SDL2
    setup_sdl2()
    
    # Setup CEF
    setup_cef()
    download_cef()
    configure_cef_libraries()
    
    # Setup platform-specific dependencies
    setup_platform_dependencies()
    
    message(STATUS "All dependencies setup complete")
endfunction()