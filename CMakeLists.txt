cmake_minimum_required(VERSION 3.20)
project(XShade VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    set(CMAKE_SYSTEM_VERSION 10.0.22000)  # Windows 11
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# DirectX libraries
set(DIRECTX_LIBS
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

# Windows libraries
set(WINDOWS_LIBS
    kernel32.lib
    user32.lib
    gdi32.lib
    shell32.lib
    comctl32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    advapi32.lib
    winmm.lib
    psapi.lib
)

# Include directories
include_directories(src/include)

# Source files for main executable
set(XSHADE_SOURCES
    src/main.cpp
    src/application.cpp
    src/ui/system_tray.cpp
    src/ui/settings_dialog.cpp
    src/config/configuration.cpp
    src/logging/logger.cpp
    src/injection/process_manager.cpp
)

# Source files for DLL
set(XSHADE_DLL_SOURCES
    src/dll/xshade_core.cpp
    src/dll/dx_hook.cpp
    src/dll/shader_manager.cpp
    src/dll/rtx_renderer.cpp
    src/dll/shader_compiler.cpp
)

# Header files
set(XSHADE_HEADERS
    src/include/application.h
    src/include/ui/system_tray.h
    src/include/ui/settings_dialog.h
    src/include/config/configuration.h
    src/include/logging/logger.h
    src/include/injection/process_manager.h
    src/include/dll/xshade_core.h
    src/include/dll/dx_hook.h
    src/include/dll/shader_manager.h
    src/include/dll/rtx_renderer.h
    src/include/dll/shader_compiler.h
    src/include/common/types.h
)

# Main executable
add_executable(XShade ${XSHADE_SOURCES} ${XSHADE_HEADERS})
target_link_libraries(XShade ${WINDOWS_LIBS})

# DLL for injection
add_library(XShadeCore SHARED ${XSHADE_DLL_SOURCES} ${XSHADE_HEADERS})
target_link_libraries(XShadeCore ${DIRECTX_LIBS} ${WINDOWS_LIBS})

# Resource files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/xshade.rc")
    target_sources(XShade PRIVATE resources/xshade.rc)
endif()

# Copy shaders to output directory
file(GLOB_RECURSE SHADER_FILES "shaders/*.hlsl")
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

# Copy configuration files
configure_file(config/xshade.ini ${CMAKE_BINARY_DIR}/xshade.ini COPYONLY)

# Set output directories
set_target_properties(XShade PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(XShadeCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows-specific settings
if(WIN32)
    set_target_properties(XShade PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
