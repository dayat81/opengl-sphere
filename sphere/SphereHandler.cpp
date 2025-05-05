#include "SphereHandler.h"

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
SphereHandler::SphereHandler(float sphereRadius)
    : mesh(sphereRadius)
    , spawner(2.0f, 1.0f)
    , gen(rd())
    , colorDist(0.0f, 1.0f)
    , gravity(9.81f)
    , floorY(-1.0f)
    , bounceFactor(0.7f)
    , sphereRadius(sphereRadius)
{
    // Create initial set of spheres with random colors
    for (int i = 0; i < 10; ++i) {
        glm::vec3 color(colorDist(gen), colorDist(gen), colorDist(gen));
        spheres.push_back(spawner.createSphere(sphereRadius, color));
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
    // Update physics for all spheres
    for (auto& sphere : spheres) {
        sphere.update(deltaTime, gravity, bounceFactor, floorY);
    }

    // Handle sphere spawning
    if (spawner.shouldSpawn(deltaTime)) {
        glm::vec3 color(colorDist(gen), colorDist(gen), colorDist(gen));
        spheres.push_back(spawner.createSphere(sphereRadius, color));
    }
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