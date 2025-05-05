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
    // Remember to first initialize GLFW.
    if (GLFW_TRUE != glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    // Standard set of window hints, I recommend you use at least OpenGL 4.0+ core.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    // For modern OpenGL, you always want to use a core profile.
    glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Technically only needed for Apple.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    // Highly recommend you use this.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    // Creates the window and checks if it succeeded by checking if window is not a nullptr.
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "OpenGL Example", nullptr,
                                          nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // This is needed for OpenGL, this will make the current OpenGL context (which contains the state) the target for
    // any operations we perform using OpenGL.
    glfwMakeContextCurrent(window);

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
    // Don't forget to call this, it destroys any resources GLFW made.
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
    // Remember to first initialize GLEW. You must always set glewExperimental to true for usage with modern OpenGL.
    glewExperimental = GL_TRUE;
    if (const auto& result = glewInit(); GLEW_OK != result)
        throw std::runtime_error("Failed to initialize GLEW");

    // Setup debug messages for OpenGL, this will tell you whenever you fucked up.
    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
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
    // Creates a new shader module.
    const GLuint module = glCreateShader(type);

    const auto& src = source.c_str();
    // This call sets the source code for a shader module.
    glShaderSource(module, 1, &src, nullptr);
    // This call actually compiles the shader module.
    glCompileShader(module);

    // This code checks for compilation errors. If there were errors, runtime error is thrown with the error message.
    GLint status;
    glGetShaderiv(module, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint size;
        glGetShaderiv(module, GL_INFO_LOG_LENGTH, &size);
        char log[size];
        glGetShaderInfoLog(module, size, nullptr, log);
        throw std::runtime_error("Failed to compile GL shader module.\n" + std::string(log));
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
    // Creates a new shader program.
    const GLuint program = glCreateProgram();

    // These calls attach shader modules to a shader program which will be used when linking.
    glAttachShader(program, vertexModule);
    glAttachShader(program, fragmentModule);

    // Links the shader modules together to create a shader program.
    glLinkProgram(program);

    // Same as the code in createShaderModule but checks for linking errors this time.
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
        char log[size];
        glGetProgramInfoLog(program, size, nullptr, log);
        throw std::runtime_error("Failed to link GL shader program.\n" + std::string(log));
    }

    return program;
}
