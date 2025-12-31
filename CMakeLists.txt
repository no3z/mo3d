cmake_minimum_required(VERSION 3.20)
project(Mo3D VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(MO3D_BUILD_TESTS "Build tests" OFF)
option(MO3D_USE_VULKAN "Use Vulkan renderer" ON)

# Find packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(RtMidi CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

if(MO3D_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    add_compile_definitions(MO3D_VULKAN)
endif()

# Source files
set(MO3D_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Window.cpp
    src/core/Time.cpp
    src/core/Settings.cpp
    src/core/ProjectManager.cpp
    src/midi/MidiInput.cpp
    src/midi/MidiMapping.cpp
    src/midi/MidiEvent.cpp
    src/scene/Scene.cpp
    src/scene/SceneObject.cpp
    src/scene/Camera.cpp
    src/scene/Transform.cpp
    src/render/Renderer.cpp
    src/render/Shader.cpp
    src/render/Mesh.cpp
    src/render/Texture.cpp
    src/render/Material.cpp
    src/render/FontRenderer.cpp
    src/ui/UIManager.cpp
    src/ui/MappingEditor.cpp
    src/ui/FileDialog.cpp
    src/utils/Logger.cpp
    src/utils/FileIO.cpp
)

set(MO3D_HEADERS
    src/core/Application.h
    src/core/Window.h
    src/core/Time.h
    src/core/Settings.h
    src/core/ProjectManager.h
    src/midi/MidiInput.h
    src/midi/MidiMapping.h
    src/midi/MidiEvent.h
    src/scene/Scene.h
    src/scene/SceneObject.h
    src/scene/Camera.h
    src/scene/Transform.h
    src/render/Renderer.h
    src/render/Shader.h
    src/render/Mesh.h
    src/render/Texture.h
    src/render/Material.h
    src/render/FontRenderer.h
    src/render/GLHeaders.h
    src/ui/UIManager.h
    src/ui/MappingEditor.h
    src/ui/FileDialog.h
    src/utils/Logger.h
    src/utils/FileIO.h
)

# Create executable
add_executable(mo3d ${MO3D_SOURCES} ${MO3D_HEADERS})

# Include directories
target_include_directories(mo3d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(mo3d PRIVATE
    glfw
    glm::glm
    imgui::imgui
    nlohmann_json::nlohmann_json
    ${FREETYPE_LIBRARIES}
    RtMidi::rtmidi
    OpenGL::GL
)

if(MO3D_USE_VULKAN)
    target_link_libraries(mo3d PRIVATE Vulkan::Vulkan)
endif()

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    target_compile_definitions(mo3d PRIVATE GL_SILENCE_DEPRECATION)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(mo3d PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif(UNIX)
    # Linux specific settings
    target_link_libraries(mo3d PRIVATE pthread dl)
    # X11 is often needed for GLFW on Linux
    find_package(X11)
    if(X11_FOUND)
        target_link_libraries(mo3d PRIVATE ${X11_LIBRARIES})
    endif()
endif()

# Install
install(TARGETS mo3d RUNTIME DESTINATION bin)
install(DIRECTORY shaders/ DESTINATION share/mo3d/shaders)
install(DIRECTORY assets/ DESTINATION share/mo3d/assets)
