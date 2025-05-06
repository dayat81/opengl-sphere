#pragma once

#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SphereMesh.h"

// External declaration of global log file
extern std::ofstream sphereHandlerLogFile;

/**
 * @brief Central manager for sphere simulation and rendering
 * 
 * This class coordinates all sphere-related functionality:
 * - Physics simulation for multiple spheres
 * - Sphere creation and lifecycle management
 * - Mesh data for rendering
 * - Color generation for visual variety
 * 
 * Acts as a facade for the sphere subsystem, providing a simple
 * interface for the main application while managing the complexity
 * of multiple interacting components.
 */
class SphereHandler {
public:
    /**
     * @brief Construct a new Sphere Handler
     * 
     * Initializes the simulation with:
     * - Shared sphere mesh for rendering
     * - Physics parameters (gravity, bounce)
     * - Initial set of spheres
     * 
     * @param sphereRadius Radius for all spheres in the simulation
     */
    SphereHandler(float sphereRadius = 0.2f);

    /**
     * @brief Update all spheres for one time step
     * 
     * Performs per-frame updates:
     * 1. Updates physics for all existing spheres
     * 2. Spawns new spheres if needed
     * 3. Handles any necessary cleanup
     * 
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Get all active spheres in the simulation
     * @return Const reference to vector of spheres
     */
    const std::vector<Sphere>& getSpheres() const;

    /**
     * @brief Get the shared sphere mesh data
     * @return Const reference to sphere mesh
     */
    const SphereMesh& getMesh() const;

    /**
     * @brief Get total number of collisions across all spheres
     * @return Sum of collision counts from all spheres
     */
    int getCollisionCount() const;

    /**
     * @brief Get number of spheres currently in simulation
     * @return Current size of sphere collection
     */
    size_t getActiveObjectCount() const { return spheres.size(); }

    /**
     * @brief Add a new sphere to the simulation
     * @param color RGB color of the new sphere
     */
    void addSphere(const glm::vec3& color);

private:
    std::vector<Sphere> spheres;      // Collection of active spheres
    SphereMesh mesh;                  // Shared mesh data for rendering
    
    std::random_device rd;            // Hardware random number source
    std::mt19937 gen;                 // Random number generator
    std::uniform_real_distribution<float> colorDist;  // For random colors

    // Physics simulation parameters
    float gravity;              // Gravity acceleration (units/s²)
    float floorY;               // Floor plane Y coordinate
    float bounceFactor;         // Energy retention in collisions
    float sphereRadius;         // Radius for all spheres
}; 