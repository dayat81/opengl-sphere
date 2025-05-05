#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>

#include "Debug.h"

// Error callback for GLFW
static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

/**
 * @file Util.h
 * @brief OpenGL initialization and utility functions
 * 
 * This file provides helper functions for:
 * - GLFW window creation and management
 * - GLEW initialization
 * - Shader compilation and linking
 * - Error checking and reporting
 */

/**
 * @brief Create and initialize a GLFW window
 * 
 * Creates a window with OpenGL context configured for:
 * - Core profile (modern OpenGL)
 * - Double buffering
 * - sRGB color space
 * - Depth and stencil buffers
 * 
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @return GLFWwindow* Pointer to created window
 * @throw runtime_error if window creation fails
 */
inline GLFWwindow* createWindow(const uint32_t width, const uint32_t height) {
    std::cout << "Setting up GLFW error callback..." << std::endl;
    glfwSetErrorCallback(glfwErrorCallback);

    std::cout << "Initializing GLFW..." << std::endl;
    if (GLFW_TRUE != glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        throw std::runtime_error("Failed to initialize GLFW");
    }
    std::cout << "GLFW initialized successfully" << std::endl;

    // Configure GLFW
    std::cout << "Configuring GLFW window hints..." << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // Enable MSAA
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    // Create window
    std::cout << "Creating GLFW window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "OpenGL Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    std::cout << "GLFW window created successfully" << std::endl;

    // Make OpenGL context current
    std::cout << "Making OpenGL context current..." << std::endl;
    glfwMakeContextCurrent(window);
    std::cout << "OpenGL context made current" << std::endl;

    // Enable vsync
    glfwSwapInterval(1);

    return window;
}

/**
 * @brief Clean up GLFW window and terminate GLFW
 * 
 * Performs proper cleanup:
 * 1. Destroys window and context
 * 2. Terminates GLFW
 * 
 * @param window Window to destroy
 */
inline void destroyWindow(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

/**
 * @brief Initialize GLEW (OpenGL Extension Wrangler)
 * 
 * Sets up GLEW for:
 * - Modern OpenGL function loading
 * - Extension support checking
 * - Core profile compatibility
 * 
 * @throw runtime_error if GLEW initialization fails
 */
inline void initGlew() {
    std::cout << "Initializing GLEW..." << std::endl;
    glewExperimental = GL_TRUE;
    if (const auto& result = glewInit(); GLEW_OK != result) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(result) << std::endl;
        throw std::runtime_error("Failed to initialize GLEW: " + std::string((const char*)glewGetErrorString(result)));
    }
    std::cout << "GLEW initialized successfully" << std::endl;

    // Setup debug messages for OpenGL
    std::cout << "Setting up OpenGL debug output..." << std::endl;
    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::cout << "OpenGL debug output enabled" << std::endl;
    } else {
        std::cout << "OpenGL debug output not supported" << std::endl;
    }

    // Print OpenGL version info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
}

/**
 * @brief Create and compile a shader module
 * 
 * Handles the process of:
 * 1. Creating shader object
 * 2. Setting source code
 * 3. Compiling shader
 * 4. Checking for compilation errors
 * 
 * @param type Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
 * @param source GLSL source code for the shader
 * @return GLuint Compiled shader module ID
 * @throw runtime_error if compilation fails
 */
inline GLuint createShaderModule(const GLenum type, const std::string& source) {
    const GLuint module = glCreateShader(type);
    const auto& src = source.c_str();
    glShaderSource(module, 1, &src, nullptr);
    glCompileShader(module);

    GLint status;
    glGetShaderiv(module, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint size;
        glGetShaderiv(module, GL_INFO_LOG_LENGTH, &size);
        std::vector<char> log(size);
        glGetShaderInfoLog(module, size, nullptr, log.data());
        throw std::runtime_error("Failed to compile GL shader module.\n" + std::string(log.data()));
    }

    return module;
}

/**
 * @brief Link shader modules into a program
 * 
 * Creates a shader program by:
 * 1. Creating program object
 * 2. Attaching shader modules
 * 3. Linking program
 * 4. Checking for link errors
 * 
 * @param vertexShader Vertex shader module ID
 * @param fragmentShader Fragment shader module ID
 * @return GLuint Linked program ID
 * @throw runtime_error if linking fails
 */
inline GLuint linkModules(const GLuint vertexModule, const GLuint fragmentModule) {
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexModule);
    glAttachShader(program, fragmentModule);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
        std::vector<char> log(size);
        glGetProgramInfoLog(program, size, nullptr, log.data());
        throw std::runtime_error("Failed to link GL shader program.\n" + std::string(log.data()));
    }

    return program;
}
