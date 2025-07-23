# MikoMacros.cmake - Reusable CMake Functions and Macros
# This file contains custom macros and functions for the MikoView project

# =============================================================================
# Target Property Macros (from CEF)
# =============================================================================

# Set common library target properties
macro(SET_LIBRARY_TARGET_PROPERTIES target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    
    # Platform-specific properties
    if(WIN32)
        set_target_properties(${target} PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
        )
    endif()
endmacro()

# Set common executable target properties
macro(SET_EXECUTABLE_TARGET_PROPERTIES target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    
    # Platform-specific properties
    if(WIN32)
        set_target_properties(${target} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
endmacro()

# =============================================================================
# Configuration Generation
# =============================================================================

# Generate app_config files from templates
function(generate_app_config)
    # Set default values if not provided
    if(NOT DEFINED MIKO_DEFAULT_WINDOW_WIDTH)
        set(MIKO_DEFAULT_WINDOW_WIDTH 1200)
    endif()
    
    if(NOT DEFINED MIKO_DEFAULT_WINDOW_HEIGHT)
        set(MIKO_DEFAULT_WINDOW_HEIGHT 800)
    endif()
    
    if(NOT DEFINED MIKO_START_HIDDEN)
        set(MIKO_START_HIDDEN true)
    endif()
    
    if(NOT DEFINED MIKO_DEBUG_PORT)
        set(MIKO_DEBUG_PORT 9222)
    endif()
    
    if(NOT DEFINED MIKO_ENABLE_HOT_RELOAD)
        set(MIKO_ENABLE_HOT_RELOAD true)
    endif()
    
    if(NOT DEFINED MIKO_DEV_SERVER_URL)
        set(MIKO_DEV_SERVER_URL "http://localhost:3000")
    endif()
    
    # Convert boolean values to C++ boolean literals
    if(MIKO_START_HIDDEN)
        set(MIKO_START_HIDDEN_BOOL "true")
    else()
        set(MIKO_START_HIDDEN_BOOL "false")
    endif()
    
    if(MIKO_ENABLE_HOT_RELOAD)
        set(MIKO_ENABLE_HOT_RELOAD_BOOL "true")
    else()
        set(MIKO_ENABLE_HOT_RELOAD_BOOL "false")
    endif()
    
    # Set platform and architecture info
    if(NOT DEFINED MIKO_PLATFORM)
        if(WIN32)
            set(MIKO_PLATFORM "Windows")
        elseif(APPLE)
            set(MIKO_PLATFORM "macOS")
        elseif(UNIX)
            set(MIKO_PLATFORM "Linux")
        else()
            set(MIKO_PLATFORM "Unknown")
        endif()
    endif()
    
    if(NOT DEFINED MIKO_ARCH)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(MIKO_ARCH "x64")
        else()
            set(MIKO_ARCH "x86")
        endif()
    endif()
    
    # Set CEF version if available
    if(NOT DEFINED CEF_VERSION)
        set(CEF_VERSION "Unknown")
    endif()
    
    # Generate header file
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/mikoview/config/app_config.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/mikoview/app_config.hpp"
        @ONLY
    )
    
    # Generate source file
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/mikoview/config/app_config.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/mikoview/app_config.cpp"
        @ONLY
    )
    
    message(STATUS "Generated app_config files with:")
    message(STATUS "  - Window Size: ${MIKO_DEFAULT_WINDOW_WIDTH}x${MIKO_DEFAULT_WINDOW_HEIGHT}")
    message(STATUS "  - Debug Port: ${MIKO_DEBUG_PORT}")
    message(STATUS "  - Dev Server: ${MIKO_DEV_SERVER_URL}")
    message(STATUS "  - Platform: ${MIKO_PLATFORM} (${MIKO_ARCH})")
endfunction()

# =============================================================================
# File Copy Utilities
# =============================================================================

# Copy files to target directory
macro(COPY_FILES target file_list source_dir target_dir)
    foreach(file_name ${file_list})
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${source_dir}/${file_name}"
            "${target_dir}/${file_name}"
            COMMENT "Copying ${file_name} to ${target_dir}"
        )
    endforeach()
endmacro()

# Copy directory contents
function(copy_directory_contents target source_dir target_dir)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${source_dir}"
        "${target_dir}"
        COMMENT "Copying directory ${source_dir} to ${target_dir}"
    )
endfunction()

# =============================================================================
# CEF-specific Macros
# =============================================================================

# Copy CEF binaries and resources
function(setup_cef_files target)
    if(OS_WINDOWS)
        # Copy CEF binary files
        COPY_FILES("${target}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
        
        # Copy CEF resource files
        COPY_FILES("${target}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
        
        # Copy SDL2 DLL to output directory
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:${target}>
            COMMENT "Copying SDL2 DLL to output directory"
        )
        
    elseif(OS_LINUX)
        # Copy CEF binary files for Linux
        COPY_FILES("${target}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
        
        # Copy CEF resource files for Linux
        COPY_FILES("${target}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
    endif()
endfunction()

# Copy application assets for release builds
function(setup_app_assets target)
    # Create assets directory
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CEF_TARGET_OUT_DIR}/assets"
        COMMENT "Creating assets directory"
    )
    
    # Copy app.zip if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/example/assets/app.zip")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/example/assets/app.zip"
            "${CEF_TARGET_OUT_DIR}/assets/app.zip"
            COMMENT "Copying app.zip to assets directory"
        )
    endif()
    
    # Copy built frontend assets if they exist
    if(EXISTS "${CMAKE_SOURCE_DIR}/renderer/react/dist")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/renderer/react/dist"
            "${CEF_TARGET_OUT_DIR}/assets"
            COMMENT "Copying React build to assets directory"
        )
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/renderer/vue/dist")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/renderer/vue/dist"
            "${CEF_TARGET_OUT_DIR}/assets"
            COMMENT "Copying Vue build to assets directory"
        )
    endif()
endfunction()

# =============================================================================
# Debug and Information Macros
# =============================================================================

# Print CEF configuration information
macro(PRINT_CEF_CONFIG)
    message(STATUS "=== CEF Configuration ===")
    message(STATUS "CEF Version: ${CEF_VERSION}")
    message(STATUS "CEF Platform: ${CEF_PLATFORM}")
    message(STATUS "CEF Root: ${CEF_ROOT}")
    message(STATUS "CEF Target Output: ${CEF_TARGET_OUT_DIR}")
    message(STATUS "=========================")
endmacro()

# Print project configuration
function(print_project_config)
    message(STATUS "=== Project Configuration ===")
    message(STATUS "Project: ${PROJECT_NAME}")
    message(STATUS "Version: ${MIKO_VERSION}")
    message(STATUS "Platform: ${MIKO_PLATFORM}")
    message(STATUS "Architecture: ${MIKO_ARCH}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
    message(STATUS "=============================")
endfunction()

# =============================================================================
# Framework Setup Utilities
# =============================================================================

# Setup MikoView framework target
function(setup_mikoview_framework)
    # Generate configuration files first
    generate_app_config()
    
    # Get platform-specific sources
    get_platform_sources(PLATFORM_SOURCES)
    
    # Create framework library
    add_library(mikoview_framework STATIC
        mikoview.cpp
        mikoview/mikoapp.cpp
        mikoview/mikoclient.cpp
        mikoview/logger.cpp
        mikoview/jsapi/invoke.cpp
        mikoview/jsapi/filesystem.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generated/mikoview/app_config.cpp
        ${PLATFORM_SOURCES}
    )
    
    # Set target properties
    SET_LIBRARY_TARGET_PROPERTIES(mikoview_framework)
    
    # Set platform-specific compiler flags
    set_platform_compiler_flags(mikoview_framework)
    
    # Framework include directories
    target_include_directories(mikoview_framework PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${CEF_ROOT}
    )
    
    # Platform-specific include directories
    if(MIKO_PLATFORM_LINUX)
        target_include_directories(mikoview_framework PRIVATE ${X11_INCLUDE_DIRS})
    endif()
    
    # Link framework libraries
    target_link_libraries(mikoview_framework PUBLIC
        SDL2::SDL2
        libcef_lib
        libcef_dll_wrapper
        ${CEF_STANDARD_LIBS}
    )
    
    # Platform-specific libraries
    if(MIKO_PLATFORM_LINUX)
        target_link_libraries(mikoview_framework PUBLIC ${X11_LIBRARIES})
    endif()
endfunction()