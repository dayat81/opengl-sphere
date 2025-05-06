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
    , bounceFactor(0.8f)
    , sphereRadius(sphereRadius) {
    if (sphereHandlerLogFile.is_open()) {
        sphereHandlerLogFile << "\n=== SphereHandler Initialized ===\n"
                           << "Sphere Radius: " << std::fixed << std::setprecision(2) << sphereRadius << "\n"
                           << "Gravity: " << gravity << "\n"
                           << "Bounce Factor: " << bounceFactor << "\n"
                           << "Floor Y: " << floorY << "\n"
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
    static float totalTime = glfwGetTime();
    
    // Update all spheres
    for (auto& sphere : spheres) {
        sphere.update(deltaTime, gravity, bounceFactor, floorY);
    }

    // Remove stationary spheres and log their removal
    auto it = std::remove_if(spheres.begin(), spheres.end(),
        [this](const Sphere& sphere) {
            if (sphere.isStationaryFor(2.0f)) {
                if (sphereHandlerLogFile.is_open()) {
                    sphereHandlerLogFile << "\n=== Ball #" << sphere.getId() << " Removed ===\n"
                                       << "Time: " << formatTime(glfwGetTime()) << "\n"
                                       << "Final Position: " << formatPosition(sphere.getPosition()) << "\n"
                                       << "Stationary Duration: " << formatTime(sphere.getLastMovementTime()) << "s\n"
                                       << "Total Collisions: " << sphere.getCollisionCount() << "\n"
                                       << "============================\n" << std::endl;
                    sphereHandlerLogFile.flush();
                }
                return true;
            }
            return false;
        }
    );
    
    spheres.erase(it, spheres.end());
}

void SphereHandler::addSphere(const glm::vec3& color) {
    // Add a new sphere at a visible position above the floor
    std::uniform_real_distribution<float> xDist(-1.0f, 1.0f);    // Narrower range
    std::uniform_real_distribution<float> yDist(3.0f, 4.0f);     // Higher starting position
    std::uniform_real_distribution<float> zDist(-1.0f, 1.0f);    // Narrower range

    glm::vec3 position(xDist(gen), yDist(gen), zDist(gen));
    spheres.emplace_back(position, sphereRadius, color);
    
    if (sphereHandlerLogFile.is_open()) {
        sphereHandlerLogFile << "\n=== New Sphere Added ===\n"
                           << "Position: " << formatPosition(position) << "\n"
                           << "Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n"
                           << "Total Spheres: " << spheres.size() << "\n"
                           << "========================\n" << std::endl;
        sphereHandlerLogFile.flush();
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