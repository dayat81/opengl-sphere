#include "SphereSpawner.h"

/**
 * @brief Implementation of sphere spawning system
 * 
 * This file contains the logic for creating and timing new spheres
 * in the simulation. It handles random position generation and
 * ensures spheres are created at regular intervals.
 */

/**
 * @brief Initialize the spawner with timing and position parameters
 * 
 * Setup process:
 * 1. Initialize random number generator
 * 2. Set up position distribution range (-0.5 to 0.5)
 * 3. Configure spawn timing parameters
 * 
 * @note The position distribution creates a spawn area centered
 *       at the origin in the XZ plane
 */
SphereSpawner::SphereSpawner(float spawnHeight, float spawnInterval)
    : gen(rd())
    , posDist(-0.5f, 0.5f)
    , spawnTimer(0.0f)
    , spawnInterval(spawnInterval)
    , spawnHeight(spawnHeight)
{}

/**
 * @brief Create a new sphere with specified properties
 * 
 * Creation process:
 * 1. Generate random XZ position within spawn area
 * 2. Set Y position to spawn height
 * 3. Initialize sphere with given radius and color
 * 
 * @note Spheres are created with zero initial velocity,
 *       allowing gravity to naturally start their motion
 */
Sphere SphereSpawner::createSphere(float radius, const glm::vec3& color) {
    glm::vec3 position(posDist(gen), spawnHeight, posDist(gen));
    return Sphere(position, radius, color);
}

/**
 * @brief Check if enough time has passed to spawn a new sphere
 * 
 * Timing logic:
 * 1. Accumulate passed time
 * 2. Check against spawn interval
 * 3. Reset timer if interval reached
 * 
 * @note The spawn check uses a simple accumulator pattern,
 *       which could be extended to handle variable time steps
 *       or more complex spawn patterns
 */
bool SphereSpawner::shouldSpawn(float deltaTime) {
    spawnTimer += deltaTime;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        return true;
    }
    return false;
} 