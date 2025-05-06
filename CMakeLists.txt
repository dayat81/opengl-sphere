# This defines the minimum required CMake version to build this project.
cmake_minimum_required(VERSION 3.28)

# This is your project (or module, for modular projects) name.
project(OpenGL_Example)

# Instructs the compiler that we are using the C++ 23 standard.
set(CMAKE_CXX_STANDARD 23)

# Add vcpkg toolchain file
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Set GLEW paths
set(GLEW_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(GLEW_LIBRARY "C:/vcpkg/installed/x64-windows/lib/glew32.lib")
set(GLEW_DLL "C:/vcpkg/installed/x64-windows/bin/glew32.dll")

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Find NVML for NVIDIA GPU metrics
find_library(NVML_LIBRARY nvidia-ml)
if(NVML_LIBRARY)
    add_definitions(-DHAS_NVML)
endif()

# Find ADL for AMD GPU metrics
find_library(ADL_LIBRARY atiadlxx)
if(ADL_LIBRARY)
    add_definitions(-DHAS_ADL)
endif()

# Add source files from performance directory
set(PERFORMANCE_SOURCES
    performance/PerformanceMonitor.cpp
    performance/PerformanceMonitor.h
    performance/GPUMonitor.cpp
    performance/GPUMonitor.h
)

# Add source files from sphere directory
set(SPHERE_SOURCES
    sphere/Sphere.cpp
    sphere/Sphere.h
    sphere/SphereMesh.cpp
    sphere/SphereMesh.h
    sphere/SphereSpawner.cpp
    sphere/SphereSpawner.h
    sphere/SphereHandler.cpp
    sphere/SphereHandler.h
)

# Add executable
add_executable(OpenGL_Example main.cpp
    LogFiles.cpp
    Util.h
    Debug.h
    ${PERFORMANCE_SOURCES}
    ${SPHERE_SOURCES}
)

# Link libraries
target_link_libraries(OpenGL_Example 
    OpenGL::GL 
    ${GLEW_LIBRARY}
    glfw
    glm::glm
    $<$<BOOL:${NVML_LIBRARY}>:${NVML_LIBRARY}>
    $<$<BOOL:${ADL_LIBRARY}>:${ADL_LIBRARY}>
)

# Include directories
target_include_directories(OpenGL_Example PRIVATE ${GLEW_INCLUDE_DIR})

# Copy DLLs to output directory
add_custom_command(TARGET OpenGL_Example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GLEW_DLL}
    $<TARGET_FILE_DIR:OpenGL_Example>
)

