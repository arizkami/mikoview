cmake_minimum_required(VERSION 3.19)
project(MikoView)

# Include our custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include configuration first
include(MikoConfig)

# Include platform detection
include(MikoPlatform)

# Include macros and functions
include(MikoMacros)

# Include and setup dependencies
include(MikoDependencies)
setup_all_dependencies()

# Configure output directories
configure_output_directories()

# Convert icon if needed (Windows)
convert_icon_if_needed()

# Setup MikoView framework (this now generates config files)
setup_mikoview_framework()

# Example executable (if enabled)
if(MIKO_BUILD_EXAMPLES)
    if(WIN32)
        # Windows application (no console unless requested)
        if(MIKO_WIN32_CONSOLE)
            add_executable(${PROJECT_NAME}
                example/main.cpp
            )
        else()
            add_executable(${PROJECT_NAME} WIN32
                example/main.cpp
            )
        endif()
    else()
        # Linux/Unix application
        add_executable(${PROJECT_NAME}
            example/main.cpp
        )
    endif()
    
    # Add Windows resource file
    if(WIN32 AND MIKO_WIN32_MANIFEST)
        target_sources(${PROJECT_NAME} PRIVATE example/assets/app.rc)
    endif()
    
    # Set target properties
    SET_EXECUTABLE_TARGET_PROPERTIES(${PROJECT_NAME})
    
    # Set platform-specific compiler flags
    set_platform_compiler_flags(${PROJECT_NAME})
    
    # Link the example with the framework
    target_link_libraries(${PROJECT_NAME}
        mikoview_framework
        SDL2::SDL2main
    )
    
    # Include directories for example
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/example
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${CEF_ROOT}
    )
    
    # Setup CEF files and app assets
    setup_cef_files(${PROJECT_NAME})
    setup_app_assets(${PROJECT_NAME})
endif()

# Print configuration information
print_configuration_summary()
print_project_config()
PRINT_CEF_CONFIG()

# Install targets
if(MIKO_BUILD_EXAMPLES)
    install(TARGETS ${PROJECT_NAME} mikoview_framework
        RUNTIME DESTINATION ${MIKO_INSTALL_BINDIR}
        LIBRARY DESTINATION ${MIKO_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${MIKO_INSTALL_LIBDIR}
    )
else()
    install(TARGETS mikoview_framework
        LIBRARY DESTINATION ${MIKO_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${MIKO_INSTALL_LIBDIR}
    )
endif()

# Install headers for the framework
install(FILES mikoview.hpp DESTINATION ${MIKO_INSTALL_INCLUDEDIR})
install(DIRECTORY mikoview/ DESTINATION ${MIKO_INSTALL_INCLUDEDIR}/mikoview
    FILES_MATCHING PATTERN "*.hpp")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/mikoview/app_config.hpp 
    DESTINATION ${MIKO_INSTALL_INCLUDEDIR}/mikoview)

# Include CPack for packaging
include(CPack)