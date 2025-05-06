#include "SphereMesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    generateNormals(sectors, stacks);
    generateTexCoords(sectors, stacks);
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
    vertices.reserve((sectors + 1) * (stacks + 1));

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(glm::vec3(x, y, z));
        }
    }
}

void SphereMesh::generateNormals(int sectors, int stacks) {
    normals.clear();
    normals.reserve(vertices.size());

    // For a sphere, normals are just normalized vertex positions
    for (const auto& vertex : vertices) {
        normals.push_back(glm::normalize(vertex));
    }
}

void SphereMesh::generateTexCoords(int sectors, int stacks) {
    texCoords.clear();
    texCoords.reserve(vertices.size());

    for (int i = 0; i <= stacks; ++i) {
        float v = 1.0f - (float)i / stacks;
        for (int j = 0; j <= sectors; ++j) {
            float u = (float)j / sectors;
            texCoords.push_back(glm::vec2(u, v));
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
    indices.reserve(sectors * stacks * 6);

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
} 