#include "SphereMesh.h"
#include <cmath>

/**
 * @brief Implementation of UV sphere mesh generation
 * 
 * This file contains the algorithms for generating a 3D sphere mesh
 * using UV sphere topology. The mesh is created by dividing the sphere
 * into sectors (longitude) and stacks (latitude), then generating
 * vertices and triangles to form the surface.
 */

/**
 * @brief Initialize and generate sphere mesh
 * 
 * Construction process:
 * 1. Generate vertex positions using spherical coordinates
 * 2. Generate triangle indices to connect vertices
 * 
 * The mesh quality is determined by the number of sectors and stacks:
 * - More sectors = smoother around equator
 * - More stacks = smoother from pole to pole
 * 
 * @note Default resolution (16x16) provides good visual quality
 *       while maintaining reasonable performance
 */
SphereMesh::SphereMesh(float radius, int sectors, int stacks) {
    generateVertices(radius, sectors, stacks);
    generateIndices(sectors, stacks);
}

/**
 * @brief Generate vertex positions for UV sphere
 * 
 * Algorithm:
 * 1. Divide sphere into stacks (latitude) and sectors (longitude)
 * 2. For each vertex:
 *    - Calculate spherical coordinates (phi, theta)
 *    - Convert to Cartesian coordinates (x, y, z)
 * 
 * Coordinate system:
 * - Y axis runs pole to pole
 * - XZ plane forms the equator
 * 
 * @note Vertices are generated in a way that ensures:
 *       - Even distribution of vertices
 *       - Proper texture coordinate mapping (if added later)
 *       - Correct normal vectors (vertex position = normal direction)
 */
void SphereMesh::generateVertices(float radius, int sectors, int stacks) {
    vertices.clear();
    
    // Generate vertices from top to bottom
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * M_PI / stacks;  // Angle from Y axis
        float xy = radius * cosf(stackAngle);             // Distance from Y axis
        float z = radius * sinf(stackAngle);              // Height (Y coordinate)
        
        // Generate vertices around the stack
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * M_PI / sectors;   // Angle around Y axis
            float x = xy * cosf(sectorAngle);             // X coordinate
            float y = xy * sinf(sectorAngle);             // Z coordinate
            vertices.push_back(glm::vec3(x, y, z));
        }
    }
}

/**
 * @brief Generate triangle indices for UV sphere
 * 
 * Algorithm:
 * 1. For each grid cell in the UV map:
 *    - Generate two triangles
 *    - Handle special cases at poles
 * 
 * Triangle generation rules:
 * - Skip first triangle in bottom stack
 * - Skip second triangle in top stack
 * - Ensure proper winding order for face culling
 * 
 * @note The index pattern creates a continuous mesh with:
 *       - No gaps between triangles
 *       - Correct vertex sharing
 *       - Proper topology at poles
 */
void SphereMesh::generateIndices(int sectors, int stacks) {
    indices.clear();
    
    // Generate triangles for each stack
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);      // First vertex in current stack
        int k2 = k1 + sectors + 1;       // First vertex in next stack
        
        // Generate triangles around the stack
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // Skip first triangle in bottom stack
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            
            // Skip second triangle in top stack
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
} 