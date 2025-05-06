#include "Sphere.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <GLFW/glfw3.h>

/**
 * @brief Implementation of Sphere physics and collision handling
 * 
 * This file contains the physics simulation logic for individual spheres,
 * including velocity updates, collision detection, and response calculations.
 */

// Initialize static ID counter
unsigned int Sphere::nextId = 0;

// External declaration of global log file
extern std::ofstream sphereLogFile;

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
    : id(nextId++)
    , position(position)
    , velocity(glm::vec3(0.0f))
    , radius(radius)
    , color(color)
    , collisionCount(0)
    , stationary(false)
    , shouldDestroy(false)
{
    if (sphereLogFile.is_open()) {
        sphereLogFile << "\n=== Ball #" << id << " Created ===\n"
                     << "Position: (" << std::fixed << std::setprecision(2) 
                     << position.x << ", " << position.y << ", " << position.z << ")\n"
                     << "Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n"
                     << "Radius: " << radius << "\n"
                     << "========================\n" << std::endl;
        sphereLogFile.flush();
    }
}

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
    // Update velocity with gravity
    velocity.y -= gravity * deltaTime;
    
    // Update position
    position += velocity * deltaTime;
    
    // Check for floor collision
    if (position.y - radius <= floorY) {
        position.y = floorY + radius;  // Prevent sinking into floor
        velocity.y = -velocity.y * bounceFactor;
        collisionCount++;
        
        // Check if sphere is becoming stationary
        // A sphere is considered stationary when:
        // 1. All velocity components are below threshold (0.1 units/s)
        // 2. The sphere is in contact with the floor
        if (std::abs(velocity.y) < 0.1f && std::abs(velocity.x) < 0.1f && std::abs(velocity.z) < 0.1f) {
            velocity = glm::vec3(0.0f);  // Stop the sphere
            stationary = true;
            shouldDestroy = true;  // Mark for destruction when stationary
            
            if (sphereLogFile.is_open()) {
                sphereLogFile << "\n=== Sphere Became Stationary and Will Be Destroyed ===\n"
                            << "ID: " << id << "\n"
                            << "Final Position: (" << std::fixed << std::setprecision(2)
                            << position.x << ", " << position.y << ", " << position.z << ")\n"
                            << "Collision Count: " << collisionCount << "\n"
                            << "==============================\n" << std::endl;
                sphereLogFile.flush();
            }
        }
    }
    
    // Handle wall collisions
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
    const float wallDistance = 5.0f;  // Distance from center to walls
    const float minVelocity = 0.1f;   // Increased minimum velocity threshold
    const float collisionCooldown = 0.1f;  // Cooldown period between collisions
    
    static float lastCollisionTime = 0.0f;
    float currentTime = glfwGetTime();
    
    // Skip collision counting if we're in cooldown
    bool canCountCollision = (currentTime - lastCollisionTime) > collisionCooldown;
    
    // Floor collision
    if (position.y - radius < floorY) {
        position.y = floorY + radius;
        velocity.y = -velocity.y * bounceFactor;
        
        // Only count collision if velocity is significant and cooldown has passed
        if (glm::length(velocity) > minVelocity && canCountCollision) {
            collisionCount++;
            lastCollisionTime = currentTime;
            if (sphereLogFile.is_open()) {
                sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                            << "Position: (" << std::fixed << std::setprecision(2)
                            << position.x << ", " << position.y << ", " << position.z << ")\n"
                            << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                            << "==============================\n" << std::endl;
                sphereLogFile.flush();
            }
        }
    }
    
    // Wall collisions - only count one collision per frame
    bool wallCollision = false;
    
    if (position.x - radius < -wallDistance) {
        position.x = -wallDistance + radius;
        velocity.x = -velocity.x * bounceFactor;
        wallCollision = true;
    }
    if (position.x + radius > wallDistance) {
        position.x = wallDistance - radius;
        velocity.x = -velocity.x * bounceFactor;
        wallCollision = true;
    }
    if (position.z - radius < -wallDistance) {
        position.z = -wallDistance + radius;
        velocity.z = -velocity.z * bounceFactor;
        wallCollision = true;
    }
    if (position.z + radius > wallDistance) {
        position.z = wallDistance - radius;
        velocity.z = -velocity.z * bounceFactor;
        wallCollision = true;
    }
    
    // Count wall collision if significant and cooldown has passed
    if (wallCollision && glm::length(velocity) > minVelocity && canCountCollision) {
        collisionCount++;
        lastCollisionTime = currentTime;
        if (sphereLogFile.is_open()) {
            sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                        << "Position: (" << std::fixed << std::setprecision(2)
                        << position.x << ", " << position.y << ", " << position.z << ")\n"
                        << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                        << "==============================\n" << std::endl;
            sphereLogFile.flush();
        }
    }
} 