/**
 * @file main.cpp
 * @brief OpenGL-based 3D sphere physics simulation
 * 
 * This application demonstrates:
 * - Modern OpenGL rendering techniques
 * - Real-time physics simulation
 * - Performance monitoring
 * - Object-oriented design patterns
 * 
 * The application creates a 3D environment where spheres:
 * - Spawn at regular intervals
 * - Fall under gravity
 * - Bounce off walls and floor
 * - Render with proper depth and lighting
 */

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <random>

#include "Util.h"
#include "sphere/SphereHandler.h"
#include "performance/PerformanceMonitor.h"

// Window dimensions for rendering
constexpr uint32_t width = 720;
constexpr uint32_t height = 480;

/**
 * @brief Main application entry point
 * 
 * Program flow:
 * 1. Initialize OpenGL context and extensions
 * 2. Set up rendering state and shaders
 * 3. Create simulation components
 * 4. Enter main render loop
 * 5. Clean up resources
 * 
 * @return 0 on successful execution
 */
int main() {
    std::ofstream logFile("C:/geopoint/OpenGL-Sphere/build/Debug/debug.log");
    if (!logFile) {
        std::cerr << "Failed to open log file" << std::endl;
        return -1;
    }

    logFile << "Program starting..." << std::endl;

    try {
        logFile << "Starting OpenGL initialization..." << std::endl;
        
        // Initialize window and OpenGL context
        GLFWwindow* window = createWindow(width, height);
        logFile << "Window created successfully" << std::endl;
        
        initGlew();
        logFile << "GLEW initialized successfully" << std::endl;

        // Configure OpenGL state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        logFile << "OpenGL state configured" << std::endl;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glViewport(0, 0, width, height);

        // Create and compile shaders
        const GLuint vertexShader = createShaderModule(GL_VERTEX_SHADER, R"(
#version 330 core

// Vertex attributes and uniforms
layout(location = 0) in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex from model space to clip space
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)");

        const GLuint fragmentShader = createShaderModule(GL_FRAGMENT_SHADER, R"(
#version 330 core

// Fragment shader outputs and uniforms
out vec4 color;
uniform vec3 sphereColor;

void main() {
    // Output sphere color with full opacity
    color = vec4(sphereColor, 1.0);
}
)");

        // Link shader program
        const GLuint shaderProgram = linkModules(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create simulation and monitoring systems
        SphereHandler sphereHandler;
        PerformanceMonitor perfMonitor;

        // Create OpenGL buffers and vertex array
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 
            sphereHandler.getMesh().getVertices().size() * sizeof(glm::vec3), 
            sphereHandler.getMesh().getVertices().data(), 
            GL_STATIC_DRAW);

        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            sphereHandler.getMesh().getIndices().size() * sizeof(unsigned int), 
            sphereHandler.getMesh().getIndices().data(), 
            GL_STATIC_DRAW);

        GLuint vertexArrayObject;
        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        // Get uniform locations for shader parameters
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "sphereColor");

        // Create perspective projection matrix
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),           // 45-degree field of view
            static_cast<float>(width) / height,  // Aspect ratio
            0.1f,                          // Near clip plane
            100.0f                         // Far clip plane
        );

        // Create camera view matrix
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),  // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at point (origin)
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );

        // Initialize timing variables
        auto lastTime = glfwGetTime();
        float metricsPrintTimer = 0.0f;
        const float metricsPrintInterval = 1.0f;  // Print metrics every second

        // Main render loop
        while (!glfwWindowShouldClose(window)) {
            perfMonitor.beginFrame();

            // Update timing
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // Handle keyboard input
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
                // Add a new sphere with random color
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
                glm::vec3 color(colorDist(gen), colorDist(gen), colorDist(gen));
                sphereHandler.addSphere(color);
            }

            // Update metrics display timer
            metricsPrintTimer += deltaTime;
            if (metricsPrintTimer >= metricsPrintInterval) {
                metricsPrintTimer = 0.0f;
                perfMonitor.logMetrics();
            }

            // Update physics simulation
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.beginRenderPass("Physics");
            }
            sphereHandler.update(deltaTime);
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.endRenderPass("Physics");
            }

            // Process window events and clear buffers
            glfwPollEvents();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shaderProgram);
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.trackShaderSwitch();
            }

            // Begin rendering
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.beginRenderPass("Render");
            }

            // Set camera and projection uniforms
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.trackStateChange(); // for uniform updates
            }

            // Render all spheres
            glBindVertexArray(vertexArrayObject);
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.trackBufferBinding();
            }
            for (const auto& sphere : sphereHandler.getSpheres()) {
                // Create model matrix for sphere position
                glm::mat4 model = glm::translate(glm::mat4(1.0f), sphere.getPosition());
                
                // Set sphere-specific uniforms
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3fv(colorLoc, 1, glm::value_ptr(sphere.getColor()));
                if (perfMonitor.isGpuMonitoringAvailable()) {
                    perfMonitor.trackStateChange(); // for per-sphere uniform updates
                }
                // Draw sphere mesh
                glDrawElements(GL_TRIANGLES, 
                             sphereHandler.getMesh().getIndexCount(), 
                             GL_UNSIGNED_INT, 
                             0);
                // Track draw call metrics
                if (perfMonitor.isGpuMonitoringAvailable()) {
                    perfMonitor.trackDrawCall(
                        sphereHandler.getMesh().getVertices().size(),
                        sphereHandler.getMesh().getIndexCount() / 3
                    );
                }
            }

            // End rendering and swap buffers
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.endRenderPass("Render");
            }
            glfwSwapBuffers(window);
            perfMonitor.endFrame();
        }

        // Cleanup OpenGL resources
        glDeleteVertexArrays(1, &vertexArrayObject);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteProgram(shaderProgram);
        destroyWindow(window);

        return 0;
    } catch (const std::exception& e) {
        logFile << "Error: " << e.what() << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        logFile << "Unknown error occurred" << std::endl;
        std::cerr << "Unknown error occurred" << std::endl;
        return -1;
    }

    logFile.close();
    return 0;
}
