#include "Sphere.h"

/**
 * @brief Implementation of Sphere physics and collision handling
 * 
 * This file contains the physics simulation logic for individual spheres,
 * including velocity updates, collision detection, and response calculations.
 */

/**
 * @brief Initialize a sphere with given properties
 * 
 * Sets up initial state:
 * - Position from parameter
 * - Zero initial velocity
 * - Radius and color from parameters
 * - Reset collision counter
 * 
 * @note Velocity starts at zero to ensure spheres begin falling from rest
 */
Sphere::Sphere(const glm::vec3& position, float radius, const glm::vec3& color)
    : position(position)
    , velocity(0.0f)
    , radius(radius)
    , color(color)
    , collisionCount(0)
{}

/**
 * @brief Update sphere physics for one simulation step
 * 
 * Physics simulation sequence:
 * 1. Apply gravitational acceleration to velocity
 * 2. Update position using current velocity
 * 3. Check and resolve collisions with boundaries
 * 
 * Uses semi-implicit Euler integration:
 * - Velocity is updated first
 * - New velocity is used to update position
 * This provides better stability than explicit Euler
 */
void Sphere::update(float deltaTime, float gravity, float bounceFactor, float floorY) {
    // Apply gravity to vertical velocity
    velocity.y -= gravity * deltaTime;
    
    // Update position using current velocity
    position += velocity * deltaTime;
    
    // Handle any collisions that occurred during movement
    handleCollisions(bounceFactor, floorY);
}

/**
 * @brief Handle collisions with world boundaries
 * 
 * Collision response process:
 * 1. Check for boundary penetration
 * 2. Move sphere back to boundary surface
 * 3. Reflect velocity with energy loss
 * 4. Increment collision counter
 * 
 * Boundaries are defined as:
 * - Floor: y = floorY
 * - Walls: x = ±1.0
 * - Walls: z = ±1.0
 * 
 * @note Collision response uses simple reflection with energy loss
 *       More complex physics (friction, etc.) could be added here
 */
void Sphere::handleCollisions(float bounceFactor, float floorY) {
    bool hadCollision = false;

    // Floor collision
    if (position.y - radius < floorY) {
        position.y = floorY + radius;  // Move to surface
        velocity.y = -velocity.y * bounceFactor;  // Reflect with energy loss
        hadCollision = true;
    }

    // Left/right wall collisions
    if (position.x - radius < -1.0f) {
        position.x = -1.0f + radius;
        velocity.x = -velocity.x * bounceFactor;
        hadCollision = true;
    }
    if (position.x + radius > 1.0f) {
        position.x = 1.0f - radius;
        velocity.x = -velocity.x * bounceFactor;
        hadCollision = true;
    }

    // Front/back wall collisions
    if (position.z - radius < -1.0f) {
        position.z = -1.0f + radius;
        velocity.z = -velocity.z * bounceFactor;
        hadCollision = true;
    }
    if (position.z + radius > 1.0f) {
        position.z = 1.0f - radius;
        velocity.z = -velocity.z * bounceFactor;
        hadCollision = true;
    }

    // Update collision statistics
    if (hadCollision) {
        collisionCount++;
    }
} 