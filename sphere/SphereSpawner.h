#pragma once
#include <random>
#include <glm/glm.hpp>
#include "Sphere.h"

/**
 * @brief Manages the creation and timing of new spheres
 * 
 * This class handles:
 * - Timed spawning of new spheres
 * - Random position generation
 * - Initial sphere configuration
 * 
 * Uses a random number generator to create varied spawn positions,
 * ensuring spheres appear at different locations within the
 * specified spawn area.
 */
class SphereSpawner {
public:
    /**
     * @brief Construct a new Sphere Spawner
     * 
     * Initializes the spawner with specified parameters:
     * - Height at which spheres spawn
     * - Time interval between spawns
     * 
     * @param spawnHeight Vertical position where spheres appear
     * @param spawnInterval Time in seconds between spawns
     */
    SphereSpawner(float spawnHeight = 2.0f, float spawnInterval = 1.0f);
    
    /**
     * @brief Create a new sphere with specified properties
     * 
     * Generates a sphere at a random horizontal position at the
     * specified spawn height. The sphere starts with zero velocity
     * and will begin falling due to gravity.
     * 
     * @param radius Radius of the new sphere
     * @param color Color of the new sphere
     * @return Newly created Sphere object
     */
    Sphere createSphere(float radius, const glm::vec3& color);

    /**
     * @brief Check if it's time to spawn a new sphere
     * 
     * Accumulates time and determines if enough time has passed
     * since the last spawn to create a new sphere.
     * 
     * @param deltaTime Time step in seconds
     * @return true if a new sphere should be spawned
     * @return false if not enough time has passed
     */
    bool shouldSpawn(float deltaTime);

    /**
     * @brief Reset the spawn timer
     * 
     * Useful for manual control of spawn timing or
     * synchronizing spawns with other events.
     */
    void resetTimer() { spawnTimer = 0.0f; }

private:
    std::random_device rd;                              // Hardware random number source
    std::mt19937 gen;                                   // Mersenne Twister generator
    std::uniform_real_distribution<float> posDist;      // Distribution for position randomization
    
    float spawnTimer;                                   // Time accumulator
    const float spawnInterval;                          // Time between spawns
    const float spawnHeight;                            // Height at which spheres appear
}; 