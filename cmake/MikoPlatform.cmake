# MikoPlatform.cmake - Platform Detection and Configuration
# This file handles platform-specific detection and configuration

# =============================================================================
# Platform Detection
# =============================================================================
if(WIN32)
    set(MIKO_PLATFORM "Windows")
    set(MIKO_PLATFORM_WINDOWS TRUE)
    set(MIKO_PLATFORM_LINUX FALSE)
    set(MIKO_PLATFORM_MACOS FALSE)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    set(MIKO_PLATFORM "macOS")
    set(MIKO_PLATFORM_WINDOWS FALSE)
    set(MIKO_PLATFORM_LINUX FALSE)
    set(MIKO_PLATFORM_MACOS TRUE)
    message(STATUS "Platform: macOS")
elseif(UNIX)
    set(MIKO_PLATFORM "Linux")
    set(MIKO_PLATFORM_WINDOWS FALSE)
    set(MIKO_PLATFORM_LINUX TRUE)
    set(MIKO_PLATFORM_MACOS FALSE)
    message(STATUS "Platform: Linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# =============================================================================
# Architecture Detection
# =============================================================================
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(MIKO_ARCH "x64")
    set(MIKO_ARCH_64 TRUE)
    set(MIKO_ARCH_32 FALSE)
else()
    set(MIKO_ARCH "x86")
    set(MIKO_ARCH_64 FALSE)
    set(MIKO_ARCH_32 TRUE)
endif()

message(STATUS "Architecture: ${MIKO_ARCH}")

# =============================================================================
# Platform-specific Source Files
# =============================================================================
function(get_platform_sources OUT_VAR)
    set(PLATFORM_SOURCES)
    
    if(MIKO_PLATFORM_WINDOWS)
        list(APPEND PLATFORM_SOURCES mikoview/gui/windows/win32_dark.cpp)
    elseif(MIKO_PLATFORM_LINUX)
        list(APPEND PLATFORM_SOURCES mikoview/gui/linux/x11_dark.cpp)
    endif()
    
    set(${OUT_VAR} ${PLATFORM_SOURCES} PARENT_SCOPE)
endfunction()

# =============================================================================
# Platform-specific Compiler Flags
# =============================================================================
function(set_platform_compiler_flags TARGET_NAME)
    if(MIKO_PLATFORM_WINDOWS)
        # Windows-specific flags
        target_compile_definitions(${TARGET_NAME} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
        )
        
        # Enable Unicode
        target_compile_definitions(${TARGET_NAME} PRIVATE
            UNICODE
            _UNICODE
        )
        
    elseif(MIKO_PLATFORM_LINUX)
        # Linux-specific flags
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -fPIC
        )
        
    elseif(MIKO_PLATFORM_MACOS)
        # macOS-specific flags
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
        )
    endif()
endfunction()

# =============================================================================
# Platform-specific Output Directory Configuration
# =============================================================================
function(configure_output_directories)
    # Determine the target output directory based on platform and generator
    if(CMAKE_CONFIGURATION_TYPES)
        # Multi-config generators (Visual Studio, Xcode)
        set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>" PARENT_SCOPE)
    else()
        # Single-config generators (Ninja, Make)
        set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}" PARENT_SCOPE)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR} PARENT_SCOPE)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR} PARENT_SCOPE)
    endif()
endfunction()

# =============================================================================
# Platform-specific Icon Conversion
# =============================================================================
function(convert_icon_if_needed)
    if(MIKO_PLATFORM_WINDOWS)
        set(ICON_PNG "${CMAKE_SOURCE_DIR}/example/assets/icon.png")
        set(ICON_ICO "${CMAKE_SOURCE_DIR}/example/assets/icon.ico")
        
        # Check if icon.png exists and icon.ico doesn't exist or is older
        if(EXISTS "${ICON_PNG}")
            if(NOT EXISTS "${ICON_ICO}" OR "${ICON_PNG}" IS_NEWER_THAN "${ICON_ICO}")
                find_package(Python3 COMPONENTS Interpreter)
                if(Python3_FOUND)
                    message(STATUS "Converting icon.png to icon.ico")
                    execute_process(
                        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tools/iconconvert.py" "${ICON_PNG}" "${ICON_ICO}"
                        RESULT_VARIABLE icon_convert_result
                        OUTPUT_VARIABLE icon_convert_output
                        ERROR_VARIABLE icon_convert_error
                    )
                    
                    if(NOT icon_convert_result EQUAL 0)
                        message(WARNING "Failed to convert icon: ${icon_convert_error}")
                    else()
                        message(STATUS "Icon converted successfully")
                    endif()
                else()
                    message(WARNING "Python3 not found, cannot convert icon automatically")
                endif()
            endif()
        endif()
    endif()
endfunction()