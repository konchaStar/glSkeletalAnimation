cmake_minimum_required(VERSION 3.30)
project(animation)

set(CMAKE_CXX_STANDARD 20)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw-3.4)

file(GLOB IMGUI_SOURCES
        libs/imgui/*.cpp
        libs/imgui/*.h
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        libs/imgui
        libs/imgui/backends
)

add_library(imgui_impl_glfw STATIC
        libs/imgui/backends/imgui_impl_glfw.cpp
)
add_library(imgui_impl_opengl3 STATIC
        libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_impl_glfw PUBLIC
        libs/imgui
        libs/imgui/backends
)
target_include_directories(imgui_impl_opengl3 PUBLIC
        libs/imgui
        libs/imgui/backends
)
target_link_libraries(imgui_impl_glfw PRIVATE imgui glfw)
target_link_libraries(imgui_impl_opengl3 PRIVATE imgui)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/ImGuiFileDialog/CMakeLists.txt")
    add_subdirectory(libs/ImGuiFileDialog)
    target_include_directories(ImGuiFileDialog PRIVATE libs/imgui)
else()
    message(WARNING "ImGuiFileDialog not found - file dialogs disabled")
    add_library(ImGuiFileDialog INTERFACE)
endif()

add_subdirectory(glm)
add_subdirectory(Assimp)
find_package(OpenGL REQUIRED)

add_library(glad STATIC src/glad/src/glad.c)
target_include_directories(glad PUBLIC src/glad/include)

add_executable(animation src/main.cpp
        src/Render/ShaderProgram.cpp
        src/Model/Mesh.cpp
        src/Camera/Camera.cpp
        src/Camera/Camera.h
        src/Model/Model.cpp
        src/Model/Model.h
        src/Utils/ImageLoader/stb_image.cpp
        src/Utils/ImageLoader/stb_image.h
        src/Animator/Animator.cpp
        src/Animator/Animator.h
        src/Animator/Bone.cpp
        src/Animator/Bone.h
        src/Utils/AssimpHelper/assimp_glm_helpers.h
        src/Animator/Animation.cpp
        src/Animator/Animation.h
        src/Utils/Logger.cpp
        src/Utils/Logger.h
        src/Animator/AnimationMixer.cpp
        src/Animator/AnimationMixer.h
)

target_include_directories(animation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/ImGuiFileDialog
)

target_link_libraries(animation PRIVATE
        glm
        glfw
        glad
        OpenGL::GL
        assimp::assimp
        imgui
        imgui_impl_glfw
        imgui_impl_opengl3
        ImGuiFileDialog
)
