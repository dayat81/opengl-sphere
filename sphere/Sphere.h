#pragma once
#include <glm/glm.hpp>

/**
 * @brief Represents a physical sphere in 3D space
 * 
 * This class handles the physics simulation of a sphere, including:
 * - Position and velocity tracking
 * - Gravity and collision response
 * - Visual properties (color, radius)
 * 
 * Each sphere maintains its own state and can update its physics
 * independently, making it easy to parallelize physics calculations
 * if needed in the future.
 */
class Sphere {
public:
    /**
     * @brief Construct a new Sphere object
     * 
     * @param position Initial 3D position of the sphere
     * @param radius Radius of the sphere in world units
     * @param color RGB color of the sphere (each component 0.0 to 1.0)
     */
    Sphere(const glm::vec3& position, float radius, const glm::vec3& color);
    
    /**
     * @brief Update sphere physics for one time step
     * 
     * Applies physics calculations in this order:
     * 1. Apply gravity
     * 2. Update position based on velocity
     * 3. Handle collisions with boundaries
     * 
     * @param deltaTime Time step in seconds
     * @param gravity Gravity acceleration in units/s²
     * @param bounceFactor Energy retention factor for collisions (0.0 to 1.0)
     * @param floorY Y-coordinate of the floor plane
     */
    void update(float deltaTime, float gravity, float bounceFactor, float floorY);

    /**
     * @brief Handle collisions with world boundaries
     * 
     * Checks and responds to collisions with:
     * - Floor (Y-axis boundary)
     * - Walls (X and Z axis boundaries)
     * 
     * When a collision occurs:
     * 1. Position is corrected to prevent penetration
     * 2. Velocity is reflected with energy loss
     * 3. Collision counter is incremented
     * 
     * @param bounceFactor Energy retention factor for collisions (0.0 to 1.0)
     * @param floorY Y-coordinate of the floor plane
     */
    void handleCollisions(float bounceFactor, float floorY);
    
    // Getters for sphere properties
    const glm::vec3& getPosition() const { return position; }
    const glm::vec3& getColor() const { return color; }
    float getRadius() const { return radius; }
    int getCollisionCount() const { return collisionCount; }
    void resetCollisionCount() { collisionCount = 0; }

private:
    glm::vec3 position;    // Current position in 3D space
    glm::vec3 velocity;    // Current velocity vector
    float radius;          // Sphere radius
    glm::vec3 color;       // RGB color values
    int collisionCount;    // Number of collisions since last reset
}; 