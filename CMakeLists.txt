# This defines the minimum required CMake version to build this project.
cmake_minimum_required(VERSION 3.28)

# This is your project (or module, for modular projects) name.
project(OpenGL_Example)

# Instructs the compiler that we are using the C++ 23 standard.
set(CMAKE_CXX_STANDARD 23)

# This attempts to find the OpenGL library and include directories, so that you can link against and include them.
# https://cmake.org/cmake/help/latest/module/FindOpenGL.html
find_package(OpenGL REQUIRED)

# Same as above but for glew.
# https://cmake.org/cmake/help/latest/module/FindGLEW.html
find_package(GLEW REQUIRED)

# Same as above but for GLFW
find_package(glfw3 3.3 REQUIRED)

# Same as above but for glm.
find_package(glm REQUIRED)

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

# This command adds a build target (executable or library, use add_library for libraries).
# The first word is the name of the executable and everything after that is the source files you want to include in
# this new target.
add_executable(OpenGL_Example main.cpp
        Util.h
        Debug.h
        ${PERFORMANCE_SOURCES}
        ${SPHERE_SOURCES})

# This links libraries against a target, the target being the executable we just added.
target_link_libraries(OpenGL_Example 
    OpenGL::GL 
    GLEW::glew 
    glfw 
    glm::glm
    $<$<BOOL:${NVML_LIBRARY}>:${NVML_LIBRARY}>
    $<$<BOOL:${ADL_LIBRARY}>:${ADL_LIBRARY}>
)

