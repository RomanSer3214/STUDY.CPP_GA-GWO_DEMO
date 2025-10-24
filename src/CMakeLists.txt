cmake_minimum_required(VERSION 3.15)
project(Demo_App LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Автоматичне виявлення Wayland
if(UNIX AND NOT APPLE)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(WAYLAND QUIET wayland-client)
        if(WAYLAND_FOUND)
            option(USE_WAYLAND "Use Wayland backend" ON)
            message(STATUS "Wayland detected, enabling Wayland support")
        else()
            option(USE_WAYLAND "Use Wayland backend" OFF)
            message(STATUS "Wayland not found, using X11")
        endif()
    endif()
endif()

set(SOURCES
    main.cpp
    ../imgui/imgui.cpp
    ../imgui/imgui_demo.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_tables.cpp
    ../imgui/imgui_widgets.cpp
    ../backends/imgui_impl_glfw.cpp
    ../backends/imgui_impl_opengl3.cpp
    ../src/GA.cpp
    ../src/GWO.cpp
    ../src/DrawScene.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ../imgui
    ../backends
    ../src
)

# Пошук OpenGL
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Налаштування для Linux
if(UNIX AND NOT APPLE)
    if(USE_WAYLAND)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED 
            wayland-client 
            wayland-cursor 
            wayland-egl 
            xkbcommon
        )
        
        # Перевірка GLFW з Wayland
        find_library(GLFW_LIBRARY glfw)
        if(NOT GLFW_LIBRARY)
            message(FATAL_ERROR "GLFW not found. Install glfw-wayland or glfw with Wayland support.")
        endif()
        
        target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_WAYLAND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${WAYLAND_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${WAYLAND_LIBRARIES})
        
    else()
        # X11
        find_package(X11 REQUIRED)
        find_package(glfw3 REQUIRED)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
    endif()
    
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()

# Загальні бібліотеки
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        OpenGL::GL
        ${CMAKE_THREAD_LIBS_INIT}
)

# GLFW
if(USE_WAYLAND AND UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY})
else()
    find_package(glfw3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()

# Windows
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdi32 imm32)
    if(MINGW)
        target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
    endif()
endif()

# macOS
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY}
    )
endif()

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wformat)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
