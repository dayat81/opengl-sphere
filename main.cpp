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
#include <algorithm>
#include <filesystem>
#include <iomanip>

#include "Util.h"
#include "sphere/SphereHandler.h"
#include "performance/PerformanceMonitor.h"

// Window dimensions for rendering
constexpr uint32_t width = 720;
constexpr uint32_t height = 480;

// Global log files
static std::ofstream performanceLogFile;
static std::ofstream sphereLogFile;
static std::ofstream sphereHandlerLogFile;

// Global metrics structure
struct PerformanceMetrics {
    float fps = 0.0f;
    float frameTime = 0.0f;
    float cpuTime = 0.0f;
    float gpuTime = 0.0f;
    size_t memoryUsage = 0;
    int drawCalls = 0;
    int stateChanges = 0;
    int shaderSwitches = 0;
    int textureBindings = 0;
    int bufferBindings = 0;
};

// Global GPU info structure
struct GPUInfo {
    std::string vendor;
    std::string renderer;
    size_t memoryUsed = 0;
    size_t totalMemory = 0;
    float utilization = 0.0f;
};

// Global instances
static PerformanceMetrics metrics;
static GPUInfo gpuInfo;

// Initialize all log files
void initializeLogFiles() {
    try {
        // Create logs directory if it doesn't exist
        std::filesystem::create_directories("logs");

        // Open log files in truncation mode
        performanceLogFile.open("logs/performance.log", std::ios::out | std::ios::trunc);
        sphereLogFile.open("logs/sphere_lifecycle.log", std::ios::out | std::ios::trunc);
        sphereHandlerLogFile.open("logs/sphere_handler.log", std::ios::out | std::ios::trunc);

        if (!performanceLogFile.is_open() || !sphereLogFile.is_open() || !sphereHandlerLogFile.is_open()) {
            throw std::runtime_error("Failed to open one or more log files");
        }

        // Write headers to log files
        performanceLogFile << "=== Performance Log ===\n";
        sphereLogFile << "=== Sphere Lifecycle Log ===\n";
        sphereHandlerLogFile << "=== Sphere Handler Log ===\n";
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to initialize log files: ") + e.what());
    }
}

void updatePerformanceMetrics(float deltaTime) {
    metrics.fps = 1.0f / deltaTime;
    metrics.frameTime = deltaTime * 1000.0f;  // Convert to milliseconds
    metrics.cpuTime = deltaTime * 1000.0f;    // Simplified for now
    metrics.gpuTime = 0.0f;                   // Will be updated by GPU monitoring
    metrics.memoryUsage = 0;                  // Will be updated by system monitoring
    metrics.drawCalls = 0;                    // Will be updated during rendering
    metrics.stateChanges = 0;                 // Will be updated during rendering
    metrics.shaderSwitches = 0;               // Will be updated during rendering
    metrics.textureBindings = 0;              // Will be updated during rendering
    metrics.bufferBindings = 0;               // Will be updated during rendering
}

void updateGPUMetrics() {
    // Get GPU info using OpenGL
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    
    gpuInfo.vendor = vendor ? vendor : "Unknown";
    gpuInfo.renderer = renderer ? renderer : "Unknown";
    gpuInfo.memoryUsed = 0;      // Will be updated by GPU monitoring
    gpuInfo.totalMemory = 0;     // Will be updated by GPU monitoring
    gpuInfo.utilization = 0.0f;  // Will be updated by GPU monitoring
}

void logPerformanceMetrics() {
    if (!performanceLogFile.is_open()) return;

    performanceLogFile << "\nPerformance Metrics:\n"
                      << "------------------\n"
                      << "FPS: " << std::fixed << std::setprecision(1) << metrics.fps << "\n"
                      << "Frame Time: " << std::fixed << std::setprecision(2) << metrics.frameTime << " ms\n"
                      << "CPU Time: " << std::fixed << std::setprecision(2) << metrics.cpuTime << " ms\n"
                      << "GPU Time: " << std::fixed << std::setprecision(2) << metrics.gpuTime << " ms\n"
                      << "Memory Usage: " << metrics.memoryUsage << " MB\n"
                      << "Draw Calls: " << metrics.drawCalls << "\n"
                      << "State Changes: " << metrics.stateChanges << "\n"
                      << "Shader Switches: " << metrics.shaderSwitches << "\n"
                      << "Texture Bindings: " << metrics.textureBindings << "\n"
                      << "Buffer Bindings: " << metrics.bufferBindings << "\n" << std::endl;
    performanceLogFile.flush();
}

void logGPUMetrics() {
    if (!performanceLogFile.is_open()) return;

    performanceLogFile << "\nGPU Metrics:\n"
                      << "-----------\n"
                      << "Vendor: " << gpuInfo.vendor << "\n"
                      << "GPU: " << gpuInfo.renderer << "\n"
                      << "Memory Used: " << gpuInfo.memoryUsed << " MB\n"
                      << "Total Memory: " << gpuInfo.totalMemory << " MB\n"
                      << "GPU Utilization: " << std::fixed << std::setprecision(1) << gpuInfo.utilization << "%\n" << std::endl;
    performanceLogFile.flush();
}

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
    try {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Initialize log files
        initializeLogFiles();

        // Create window
        GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Sphere Simulation", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Make the window's context current
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        glewExperimental = GL_TRUE;  // Needed for core profile
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // Configure OpenGL state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
        glViewport(0, 0, width, height);

        // Create and compile shaders
        const GLuint vertexShader = createShaderModule(GL_VERTEX_SHADER, R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = texCoord;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)");

        const GLuint fragmentShader = createShaderModule(GL_FRAGMENT_SHADER, R"(
#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 sphereColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float time;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
    // Ambient
    vec3 ambient = light.ambient * material.ambient;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // Combine results
    vec3 result = (ambient + diffuse + specular) * sphereColor;
    FragColor = vec4(result, 1.0);
}
)");

        // Link shader program
        const GLuint shaderProgram = linkModules(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Check for shader compilation errors
        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            throw std::runtime_error(std::string("Shader program linking failed: ") + infoLog);
        }

        // Create simulation and monitoring systems
        SphereHandler sphereHandler(0.2f);  // Create sphere handler with radius 0.2
        
        // Create an initial sphere with a bright color at a visible position
        glm::vec3 initialColor(1.0f, 0.5f, 0.2f);  // Bright orange color
        sphereHandler.addSphere(initialColor);
        std::cout << "Initial sphere created" << std::endl;

        // Create performance monitor
        PerformanceMonitor perfMonitor;

        // Create OpenGL buffers and vertex array
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        
        // Calculate total size needed for positions, normals, and texture coordinates
        const auto& vertices = sphereHandler.getMesh().getVertices();
        const auto& normals = sphereHandler.getMesh().getNormals();
        const auto& texCoords = sphereHandler.getMesh().getTexCoords();
        
        size_t vertexDataSize = vertices.size() * sizeof(glm::vec3);
        size_t normalDataSize = normals.size() * sizeof(glm::vec3);
        size_t texCoordDataSize = texCoords.size() * sizeof(glm::vec2);
        size_t totalSize = vertexDataSize + normalDataSize + texCoordDataSize;
        
        // Allocate buffer and copy data
        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, vertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normals.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize, texCoordDataSize, texCoords.data());

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
        
        // Bind buffers
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        // Set up vertex attributes with proper strides and offsets
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)vertexDataSize);
        glEnableVertexAttribArray(1);
        
        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(vertexDataSize + normalDataSize));
        glEnableVertexAttribArray(2);

        // Unbind VAO to prevent accidental modifications
        glBindVertexArray(0);

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

        // Create camera view matrix - moved back and up for better view
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 2.0f, 8.0f),  // Camera position - moved back and up
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at point (origin)
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );

        // Initialize timing variables
        auto lastTime = glfwGetTime();
        float metricsPrintTimer = 0.0f;
        const float metricsPrintInterval = 1.0f;  // Print metrics every second

        // Main loop
        while (!glfwWindowShouldClose(window)) {
            float currentFrame = glfwGetTime();
            float deltaTime = currentFrame - lastTime;
            lastTime = currentFrame;

            // Handle keyboard input
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
                // Generate a random color
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);  // Brighter colors
                glm::vec3 color(colorDist(gen), colorDist(gen), colorDist(gen));
                sphereHandler.addSphere(color);
            }

            // Update performance metrics
            updatePerformanceMetrics(deltaTime);
            logPerformanceMetrics();

            // Update GPU metrics
            updateGPUMetrics();
            logGPUMetrics();

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
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shaderProgram);

            // Set up OpenGL state for transparent objects
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_TRUE);

            // Begin rendering
            if (perfMonitor.isGpuMonitoringAvailable()) {
                perfMonitor.beginRenderPass("Render");
            }

            // Set camera and projection uniforms
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            // Update view position uniform
            glm::vec3 viewPos(0.0f, 2.0f, 8.0f);  // Match camera position
            glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

            // Set brighter light position and properties
            glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
            glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
            
            // Set material properties - make them brighter
            glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.3f, 0.3f, 0.3f);   // Increased ambient
            glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.9f, 0.9f, 0.9f);   // Increased diffuse
            glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 1.0f, 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 32.0f);

            // Set light properties - make them brighter
            glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), 0.3f, 0.3f, 0.3f);    // Increased ambient
            glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), 0.9f, 0.9f, 0.9f);    // Increased diffuse
            glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);

            // Sort spheres by depth for proper transparency
            std::vector<std::pair<float, const Sphere*>> sortedSpheres;
            glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);  // Match camera position from view matrix
            
            for (const auto& sphere : sphereHandler.getSpheres()) {
                float distance = glm::length(cameraPos - sphere.getPosition());
                sortedSpheres.push_back({distance, &sphere});
            }
            
            // Sort spheres by distance (back to front)
            std::sort(sortedSpheres.begin(), sortedSpheres.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });

            // Render spheres in sorted order
            glBindVertexArray(vertexArrayObject);
            for (const auto& [distance, sphere] : sortedSpheres) {
                // Create model matrix for sphere position
                glm::mat4 model = glm::translate(glm::mat4(1.0f), sphere->getPosition());
                
                // Set sphere-specific uniforms
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3fv(colorLoc, 1, glm::value_ptr(sphere->getColor()));
                
                // Calculate velocity magnitude for fade effect
                float velocityMagnitude = glm::length(sphere->getVelocity());
                glUniform1f(glGetUniformLocation(shaderProgram, "velocity"), velocityMagnitude);
                
                if (perfMonitor.isGpuMonitoringAvailable()) {
                    perfMonitor.trackStateChange();
                }
                
                // Draw sphere mesh
                glDrawElements(GL_TRIANGLES, 
                             sphereHandler.getMesh().getIndices().size(), 
                             GL_UNSIGNED_INT, 
                             0);
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
        glfwDestroyWindow(window);
        glfwTerminate();

        // Close log files
        performanceLogFile.close();
        sphereLogFile.close();
        sphereHandlerLogFile.close();

        return 0;
    } catch (const std::exception& e) {
        // Close log files before throwing
        performanceLogFile.close();
        sphereLogFile.close();
        sphereHandlerLogFile.close();
        throw;
    }
}
