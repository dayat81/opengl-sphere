#include "SphereHandler.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <GLFW/glfw3.h>

/**
 * @brief Implementation of central sphere simulation manager
 * 
 * This file contains the core logic for managing the sphere simulation,
 * coordinating between physics, spawning, and rendering components.
 * It acts as the main interface between the application and the
 * sphere subsystem.
 */

/**
 * @brief Initialize the sphere simulation system
 * 
 * Setup process:
 * 1. Create shared mesh for rendering
 * 2. Initialize sphere spawner
 * 3. Set up random color generation
 * 4. Configure physics parameters
 * 5. Create initial set of spheres
 * 
 * Physics parameters are configured for realistic behavior:
 * - Gravity: 9.81 units/s² (standard Earth gravity)
 * - Bounce: 0.7 (30% energy loss per bounce)
 * - Floor: -1.0 units (below camera view)
 */

// External declaration of global log file
extern std::ofstream sphereHandlerLogFile;
extern std::ofstream sphereLogFile;

// Helper function to format time
std::string formatTime(float time) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << time;
    return ss.str();
}

// Helper function to format position
std::string formatPosition(const glm::vec3& pos) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) 
       << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
    return ss.str();
}

SphereHandler::SphereHandler(float sphereRadius) 
    : mesh(sphereRadius, 32, 32)
    , gen(rd())
    , colorDist(0.0f, 1.0f)
    , gravity(9.81f)
    , floorY(0.0f)
    , bounceFactor(0.6f)  // Reduced bounce factor for more realistic bouncing
    , sphereRadius(sphereRadius) {
    
    // Create initial sphere with bright orange color
    glm::vec3 initialColor(1.0f, 0.5f, 0.2f);  // Bright orange
    addSphere(initialColor);
    
    if (sphereHandlerLogFile.is_open()) {
        sphereHandlerLogFile << "\n=== SphereHandler Initialized ===\n"
                           << "Sphere Radius: " << std::fixed << std::setprecision(2) << sphereRadius << "\n"
                           << "Gravity: " << gravity << "\n"
                           << "Bounce Factor: " << bounceFactor << "\n"
                           << "Floor Y: " << floorY << "\n"
                           << "Initial sphere created\n"
                           << "==============================\n" << std::endl;
        sphereHandlerLogFile.flush();
    }
}

/**
 * @brief Update the entire sphere simulation
 * 
 * Per-frame update sequence:
 * 1. Update physics for all existing spheres
 * 2. Check for and handle new sphere spawning
 * 
 * The update maintains separation of concerns:
 * - Each sphere handles its own physics
 * - Spawner manages creation timing
 * - This class coordinates the overall system
 * 
 * @note The simulation uses a fixed time step for stability,
 *       but could be extended to handle variable time steps
 */
void SphereHandler::update(float deltaTime) {
    // Update all spheres
    for (auto& sphere : spheres) {
        sphere.update(deltaTime, gravity, bounceFactor, floorY);
    }

    // Remove spheres that should be destroyed (stationary or marked for destruction)
    spheres.erase(
        std::remove_if(spheres.begin(), spheres.end(),
            [](const Sphere& sphere) { 
                return sphere.shouldBeDestroyed() || 
                       (sphere.isStationary() && sphere.getCollisionCount() > 0); 
            }),
        spheres.end()
    );
}

void SphereHandler::addSphere(const glm::vec3& color) {
    static float lastSpawnTime = 0.0f;
    float currentTime = glfwGetTime();
    const float spawnCooldown = 0.5f;  // Reduced to 0.5 seconds between spawns
    
    // For the first sphere, don't check cooldown
    if (lastSpawnTime > 0.0f) {
        // Check if enough time has passed since last spawn
        if (currentTime - lastSpawnTime < spawnCooldown) {
            return;
        }
    }
    
    // Add a new sphere at a visible position above the floor
    std::uniform_real_distribution<float> xDist(-3.0f, 3.0f);    // Even wider range
    std::uniform_real_distribution<float> yDist(3.0f, 6.0f);     // Higher starting position
    std::uniform_real_distribution<float> zDist(-3.0f, 3.0f);    // Even wider range

    glm::vec3 position(xDist(gen), yDist(gen), zDist(gen));
    spheres.emplace_back(position, sphereRadius, color);
    
    // Set initial velocity to make the sphere fall
    spheres.back().setVelocity(glm::vec3(0.0f, -2.0f, 0.0f));  // Initial velocity
    
    // Update last spawn time
    lastSpawnTime = currentTime;
    
    // Log sphere creation
    if (sphereHandlerLogFile.is_open()) {
        sphereHandlerLogFile << "\n=== New Sphere Added ===\n"
                           << "Position: " << formatPosition(position) << "\n"
                           << "Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n"
                           << "Total Spheres: " << spheres.size() << "\n"
                           << "Time until next spawn: " << spawnCooldown << " seconds\n"
                           << "========================\n" << std::endl;
        sphereHandlerLogFile.flush();
    }

    // Also log to sphere lifecycle log
    if (sphereLogFile.is_open()) {
        sphereLogFile << "\n=== New Sphere Added ===\n"
                     << "Position: " << formatPosition(position) << "\n"
                     << "Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n"
                     << "Total Spheres: " << spheres.size() << "\n"
                     << "Time until next spawn: " << spawnCooldown << " seconds\n"
                     << "========================\n" << std::endl;
        sphereLogFile.flush();
    }
}

const std::vector<Sphere>& SphereHandler::getSpheres() const {
    return spheres;
}

const SphereMesh& SphereHandler::getMesh() const {
    return mesh;
}

/**
 * @brief Calculate total collisions across all spheres
 * 
 * Aggregates collision counts from all active spheres to
 * provide a global view of simulation activity. This is
 * useful for:
 * - Performance monitoring
 * - Simulation statistics
 * - Debugging physics behavior
 * 
 * @return Total number of collisions in the simulation
 */
int SphereHandler::getCollisionCount() const {
    int totalCollisions = 0;
    for (const auto& sphere : spheres) {
        totalCollisions += sphere.getCollisionCount();
    }
    return totalCollisions;
} 