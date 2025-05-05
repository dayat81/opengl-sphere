#pragma once
#include <chrono>
#include <string>
#include <deque>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <memory>
#include "GPUMonitor.h"

// Forward declarations
class GPUMonitor;

/**
 * @brief Performance monitoring system for real-time graphics applications
 * 
 * This class provides comprehensive performance metrics including:
 * - Frame timing (FPS, frame time)
 * - Processing breakdowns (physics, rendering)
 * - Memory usage (virtual and physical)
 * - Object and collision statistics
 */
class PerformanceMonitor {
public:
    /**
     * @brief Holds performance metrics for a single frame
     * 
     * Contains timing information, object counts, and memory usage statistics:
     * - Frame timing: Total time spent processing one frame
     * - CPU timing: Time spent on CPU calculations
     * - GPU timing: Time spent on GPU calculations
     * - Memory usage: Both virtual (address space) and resident (physical RAM) memory
     * - Object counts: Number of active objects, collisions, and draw calls
     * - Render pass metrics: Number of vertices and triangles processed
     */
    struct FrameMetrics {
        double frameTime = 0.0;        // Frame time in milliseconds
        double fps = 0.0;             // Frames per second
        double cpuTime = 0.0;         // CPU time in milliseconds
        double gpuTime = 0.0;         // GPU time in milliseconds
        size_t memoryUsage = 0;       // Memory usage in bytes
        size_t drawCalls = 0;         // Number of draw calls
        size_t stateChanges = 0;      // Number of state changes
        size_t shaderSwitches = 0;    // Number of shader switches
        size_t textureBindings = 0;   // Number of texture bindings
        size_t bufferBindings = 0;    // Number of buffer bindings
        ::GPUMetrics gpuMetrics;      // GPU-specific metrics
    };

    struct RenderPassMetrics {
        std::string name;
        float gpuTime = 0.0f;
        size_t vertices = 0;
        size_t triangles = 0;
    };

    /**
     * @brief Constructs a performance monitor
     * @param historySize Number of frames to keep in history for averaging
     */
    explicit PerformanceMonitor(size_t historySize = 60);
    
    /**
     * @brief Frame timing markers
     * These functions mark the start/end of different processing phases
     */
    void beginFrame();    // Start of frame processing
    void endFrame();       // End of frame processing

    // Render pass timing
    void beginRenderPass(const std::string& passName);
    void endRenderPass(const std::string& passName);

    // Performance tracking
    void trackDrawCall(size_t vertices, size_t triangles);
    void trackStateChange();
    void trackShaderSwitch();
    void trackTextureBinding();
    void trackBufferBinding();

    /**
     * @brief Performance metric accessors
     * @return Various averaged performance metrics
     */
    const FrameMetrics& getLatestMetrics() const;
    const std::deque<FrameMetrics>& getMetricsHistory() const;
    const std::vector<RenderPassMetrics>& getRenderPassMetrics() const;
    const ::GPUMetrics& getGPUMetrics() const;

    /**
     * @brief Check if GPU monitoring is available
     * @return true if GPU monitoring is supported
     */
    bool isGpuMonitoringAvailable() const { return gpuMonitor != nullptr; }

    /**
     * @brief Output current performance metrics to console
     * Displays formatted metrics including:
     * - FPS and timing breakdowns
     * - Memory usage (Virtual and Resident)
     * - Object and collision counts
     */
    void logMetrics() const;
    void logRenderPassMetrics() const;
    void logGPUMetrics() const;

    ~PerformanceMonitor();

private:
    // High-resolution clock types for precise timing
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    // Timing markers for different processing phases
    TimePoint frameStart;
    TimePoint renderStart;
    
    // Historical data for averaging
    std::deque<FrameMetrics> metricsHistory;
    std::vector<RenderPassMetrics> renderPassMetrics;
    size_t historySize;           // Maximum history length
    mutable std::mutex metricsMutex;

    // Frame timing
    double lastFrameTime;
    double frameStartTime;
    double frameEndTime;
    float fps;
    float frameTime;

    // Render timing
    double renderStartTime;
    double renderEndTime;
    float renderTime;

    // GPU monitoring
    std::unique_ptr<GPUMonitor> gpuMonitor;

    FrameMetrics currentMetrics;

    /**
     * @brief Update historical data with new value
     * @param history Vector of historical values
     * @param value New value to add
     */
    void updateHistory(std::vector<float>& history, float value);

    /**
     * @brief Calculate average of historical values
     * @param history Vector of historical values
     * @return Average value
     */
    float calculateAverage(const std::vector<float>& history) const;

    /**
     * @brief Update memory usage statistics
     * Queries the operating system for:
     * - Virtual Memory: Total address space allocated to process
     * - Resident Memory: Actual physical memory being used
     */
    void updateMemoryMetrics();

    /**
     * @brief Format memory size in human-readable units
     * @param bytes Raw size in bytes
     * @return Formatted string with appropriate unit (B, KB, MB, GB)
     */
    std::string formatMemorySize(size_t bytes) const;

    size_t getVirtualMemory() const;
    size_t getResidentMemory() const;

    void updateMetrics();
    void updateRenderPassMetrics();
}; 