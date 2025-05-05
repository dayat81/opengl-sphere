#pragma once
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Manages the 3D mesh data for sphere rendering
 * 
 * This class handles the generation and storage of sphere geometry:
 * - Vertex positions for the sphere surface
 * - Triangle indices for efficient rendering
 * - Configurable resolution (sectors and stacks)
 * 
 * The mesh is generated using UV sphere topology:
 * - Longitude lines (sectors) divide the sphere horizontally
 * - Latitude lines (stacks) divide the sphere vertically
 * - Higher numbers of sectors/stacks create smoother spheres
 */
class SphereMesh {
public:
    /**
     * @brief Construct a new Sphere Mesh
     * 
     * Generates a UV sphere mesh with the specified parameters.
     * Default resolution (16x16) provides a good balance between
     * visual quality and performance.
     * 
     * @param radius Radius of the sphere
     * @param sectors Number of horizontal divisions (longitude)
     * @param stacks Number of vertical divisions (latitude)
     */
    SphereMesh(float radius, int sectors = 16, int stacks = 16);
    
    /**
     * @brief Get the vertex positions of the sphere mesh
     * @return Const reference to vector of 3D vertex positions
     */
    const std::vector<glm::vec3>& getVertices() const { return vertices; }

    /**
     * @brief Get the triangle indices of the sphere mesh
     * @return Const reference to vector of vertex indices
     */
    const std::vector<unsigned int>& getIndices() const { return indices; }

    /**
     * @brief Get the total number of indices in the mesh
     * @return Number of indices (3 per triangle)
     */
    size_t getIndexCount() const { return indices.size(); }

private:
    /**
     * @brief Generate vertex positions for the sphere
     * 
     * Creates vertices using spherical coordinates:
     * - Radius determines sphere size
     * - Sectors divide the sphere horizontally
     * - Stacks divide the sphere vertically
     * 
     * @param radius Sphere radius
     * @param sectors Number of horizontal divisions
     * @param stacks Number of vertical divisions
     */
    void generateVertices(float radius, int sectors, int stacks);

    /**
     * @brief Generate triangle indices for the sphere
     * 
     * Creates triangles to connect vertices:
     * - Generates two triangles per grid cell
     * - Handles pole regions specially
     * - Ensures correct winding order for face culling
     * 
     * @param sectors Number of horizontal divisions
     * @param stacks Number of vertical divisions
     */
    void generateIndices(int sectors, int stacks);

    std::vector<glm::vec3> vertices;      // 3D positions of vertices
    std::vector<unsigned int> indices;     // Triangle indices
}; 