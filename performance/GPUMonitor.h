#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include <unordered_map>

/**
 * @brief GPU monitoring system for collecting GPU metrics
 */
class GPUMonitor {
public:
    struct GPUMetrics {
        float utilizationPercent = -1.0f;  // -1 indicates not available
        size_t memoryUsedBytes = 0;    // GPU memory used in bytes
        size_t memoryTotalBytes = 0;   // Total GPU memory in bytes
        std::string gpuName;       // GPU device name
        std::string vendor;        // GPU vendor (NVIDIA, AMD, etc.)
    };

    struct RenderPassMetrics {
        float gpuTime = 0.0f;             // GPU time in milliseconds
        size_t drawCalls = 0;          // Number of draw calls
        size_t triangles = 0;          // Number of triangles rendered
        size_t vertices = 0;           // Number of vertices processed
    };

    struct PerformanceCounters {
        size_t textureBindings = 0;    // Number of texture bindings
        size_t shaderSwitches = 0;     // Number of shader program switches
        size_t bufferBindings = 0;     // Number of buffer bindings
        size_t stateChanges = 0;       // Number of state changes
        size_t drawCalls = 0;
    };

    GPUMonitor();
    ~GPUMonitor();

    /**
     * @brief Update GPU metrics
     * @return true if metrics were successfully updated
     */
    bool updateMetrics();

    /**
     * @brief Get the current GPU metrics
     * @return Current GPU metrics
     */
    const GPUMetrics& getMetrics() const { return metrics; }

    /**
     * @brief Check if GPU monitoring is available
     * @return true if GPU monitoring is supported
     */
    bool isAvailable() const { return available; }

    /**
     * @brief Begin profiling a render pass
     * @param passName Name of the render pass
     */
    void beginRenderPass(const std::string& passName);

    /**
     * @brief End profiling a render pass
     * @param passName Name of the render pass
     */
    void endRenderPass(const std::string& passName);

    /**
     * @brief Get metrics for a specific render pass
     * @param passName Name of the render pass
     * @return Render pass metrics
     */
    const RenderPassMetrics& getRenderPassMetrics(const std::string& passName) const;

    /**
     * @brief Get all render pass metrics
     * @return Map of render pass names to their metrics
     */
    const std::map<std::string, RenderPassMetrics>& getAllRenderPassMetrics() const {
        return renderPassMetrics;
    }

    /**
     * @brief Get current performance counters
     * @return Current performance counters
     */
    const PerformanceCounters& getPerformanceCounters() const { return performanceCounters; }

    /**
     * @brief Reset performance counters
     */
    void resetPerformanceCounters();

    // Render pass timing
    void trackDrawCall(size_t vertices, size_t triangles);
    void trackStateChange();
    void trackShaderSwitch();
    void trackTextureBinding();
    void trackBufferBinding();

private:
    bool available;
    GPUMetrics metrics;
    void* driverHandle;  // Opaque handle for driver-specific data
    PerformanceCounters performanceCounters;
    std::map<std::string, RenderPassMetrics> renderPassMetrics;
    std::map<std::string, GLuint> renderPassQueries;  // Timer queries for each pass
    std::string currentPass;

    bool initNVIDIA();
    bool initAMD();
    void cleanup();

    // OpenGL performance monitoring
    void initPerformanceMonitoring();
    void updatePerformanceCounters();
}; 