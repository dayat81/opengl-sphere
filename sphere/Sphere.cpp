#include "Sphere.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>

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
    , lastMovementTime(0.0f)
    , isStationary(false)
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
    
    // Handle collisions
    handleCollisions(bounceFactor, floorY);
    
    // Check if sphere is stationary
    float velocityMagnitude = glm::length(velocity);
    if (velocityMagnitude < 0.01f) {
        if (!isStationary) {
            isStationary = true;
            lastMovementTime = 0.0f;
            if (sphereLogFile.is_open()) {
                sphereLogFile << "\n=== Ball #" << id << " Became Stationary ===\n"
                            << "Final Position: (" << std::fixed << std::setprecision(2)
                            << position.x << ", " << position.y << ", " << position.z << ")\n"
                            << "Total Collisions: " << collisionCount << "\n"
                            << "================================\n" << std::endl;
                sphereLogFile.flush();
            }
        }
        lastMovementTime += deltaTime;
    } else {
        if (isStationary) {
            isStationary = false;
            if (sphereLogFile.is_open()) {
                sphereLogFile << "\n=== Ball #" << id << " Started Moving ===\n"
                            << "Position: (" << std::fixed << std::setprecision(2)
                            << position.x << ", " << position.y << ", " << position.z << ")\n"
                            << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                            << "================================\n" << std::endl;
                sphereLogFile.flush();
            }
        }
        lastMovementTime = 0.0f;
    }
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
    
    // Floor collision
    if (position.y - radius < floorY) {
        position.y = floorY + radius;
        velocity.y = -velocity.y * bounceFactor;
        collisionCount++;
        if (sphereLogFile.is_open()) {
            sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                        << "Position: (" << std::fixed << std::setprecision(2)
                        << position.x << ", " << position.y << ", " << position.z << ")\n"
                        << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                        << "==============================\n" << std::endl;
            sphereLogFile.flush();
        }
    }
    
    // Wall collisions
    if (position.x - radius < -wallDistance) {
        position.x = -wallDistance + radius;
        velocity.x = -velocity.x * bounceFactor;
        collisionCount++;
        if (sphereLogFile.is_open()) {
            sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                        << "Position: (" << std::fixed << std::setprecision(2)
                        << position.x << ", " << position.y << ", " << position.z << ")\n"
                        << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                        << "==============================\n" << std::endl;
            sphereLogFile.flush();
        }
    }
    if (position.x + radius > wallDistance) {
        position.x = wallDistance - radius;
        velocity.x = -velocity.x * bounceFactor;
        collisionCount++;
        if (sphereLogFile.is_open()) {
            sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                        << "Position: (" << std::fixed << std::setprecision(2)
                        << position.x << ", " << position.y << ", " << position.z << ")\n"
                        << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                        << "==============================\n" << std::endl;
            sphereLogFile.flush();
        }
    }
    if (position.z - radius < -wallDistance) {
        position.z = -wallDistance + radius;
        velocity.z = -velocity.z * bounceFactor;
        collisionCount++;
        if (sphereLogFile.is_open()) {
            sphereLogFile << "\n=== Ball #" << id << " Collision #" << collisionCount << " ===\n"
                        << "Position: (" << std::fixed << std::setprecision(2)
                        << position.x << ", " << position.y << ", " << position.z << ")\n"
                        << "New Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")\n"
                        << "==============================\n" << std::endl;
            sphereLogFile.flush();
        }
    }
    if (position.z + radius > wallDistance) {
        position.z = wallDistance - radius;
        velocity.z = -velocity.z * bounceFactor;
        collisionCount++;
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