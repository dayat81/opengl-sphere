#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/resource.h>
#include <unistd.h>

PerformanceMonitor::PerformanceMonitor(size_t historySize)
    : historySize(historySize)
    , frameStart(std::chrono::high_resolution_clock::now())
{
    metricsHistory.resize(historySize);
}

PerformanceMonitor::~PerformanceMonitor() {
    // Cleanup is handled by member destructors
}

void PerformanceMonitor::beginFrame() {
    frameStart = std::chrono::high_resolution_clock::now();
    gpuMonitor.resetPerformanceCounters();
}

void PerformanceMonitor::endFrame() {
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
    
    // Update metrics
    updateMetrics();
    
    // Calculate frame metrics
    currentMetrics.frameTime = frameDuration.count() / 1000.0; // Convert to milliseconds
    currentMetrics.fps = 1000.0f / currentMetrics.frameTime;
    
    // Update history
    std::lock_guard<std::mutex> lock(metricsMutex);
    metricsHistory.push_back(currentMetrics);
    if (metricsHistory.size() > historySize) {
        metricsHistory.pop_front();
    }
}

void PerformanceMonitor::beginRenderPass(const std::string& passName) {
    if (passName == "Render") {
        renderStart = std::chrono::high_resolution_clock::now();
    }
    gpuMonitor.beginRenderPass(passName);
}

void PerformanceMonitor::endRenderPass(const std::string& passName) {
    gpuMonitor.endRenderPass(passName);
    if (passName == "Render") {
        auto renderEnd = std::chrono::high_resolution_clock::now();
        double cpuTime = std::chrono::duration_cast<std::chrono::microseconds>(renderEnd - renderStart).count() / 1000.0;
        currentMetrics.cpuTime = cpuTime;
        // Try to get GPU time from render pass metrics
        updateRenderPassMetrics();
        for (const auto& pass : renderPassMetrics) {
            if (pass.name == "Render") {
                currentMetrics.gpuTime = pass.gpuTime;
                break;
            }
        }
    }
}

void PerformanceMonitor::trackDrawCall(size_t vertices, size_t triangles) {
    gpuMonitor.trackDrawCall(vertices, triangles);
}

void PerformanceMonitor::trackStateChange() {
    gpuMonitor.trackStateChange();
}

void PerformanceMonitor::trackShaderSwitch() {
    gpuMonitor.trackShaderSwitch();
}

void PerformanceMonitor::trackTextureBinding() {
    gpuMonitor.trackTextureBinding();
}

void PerformanceMonitor::trackBufferBinding() {
    gpuMonitor.trackBufferBinding();
}

const PerformanceMonitor::FrameMetrics& PerformanceMonitor::getLatestMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return metricsHistory.back();
}

const std::deque<PerformanceMonitor::FrameMetrics>& PerformanceMonitor::getMetricsHistory() const {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return metricsHistory;
}

const std::vector<PerformanceMonitor::RenderPassMetrics>& PerformanceMonitor::getRenderPassMetrics() const {
    return renderPassMetrics;
}

const GPUMonitor::GPUMetrics& PerformanceMonitor::getGPUMetrics() const {
    return gpuMonitor.getMetrics();
}

void PerformanceMonitor::logMetrics() const {
    const auto& metrics = getLatestMetrics();
    const auto& gpuMetrics = gpuMonitor.getMetrics();
    
    std::cout << "\nPerformance Metrics:\n"
              << "-------------------\n"
              << "Frame Time: " << std::fixed << std::setprecision(2) << metrics.frameTime << " ms\n"
              << "FPS: " << std::fixed << std::setprecision(1) << metrics.fps << "\n"
              << "CPU Time: " << metrics.cpuTime << " ms\n"
              << "GPU Time: " << metrics.gpuTime << " ms\n"
              << "Memory Usage: " << (metrics.memoryUsage / (1024 * 1024)) << " MB\n"
              << "Draw Calls: " << metrics.drawCalls << "\n"
              << "State Changes: " << metrics.stateChanges << "\n"
              << "Shader Switches: " << metrics.shaderSwitches << "\n"
              << "Texture Bindings: " << metrics.textureBindings << "\n"
              << "Buffer Bindings: " << metrics.bufferBindings << "\n";
    
    if (gpuMetrics.utilizationPercent >= 0) {
        std::cout << "GPU Utilization: " << std::fixed << std::setprecision(1) 
                  << gpuMetrics.utilizationPercent << "%\n";
    }
    
    if (gpuMetrics.memoryTotalBytes > 0) {
        std::cout << "GPU Memory: " << (gpuMetrics.memoryUsedBytes / (1024 * 1024)) << " MB / "
                  << (gpuMetrics.memoryTotalBytes / (1024 * 1024)) << " MB\n";
    }
}

void PerformanceMonitor::logRenderPassMetrics() const {
    std::cout << "\nRender Pass Metrics:\n"
              << "-------------------\n";
    
    for (const auto& pass : renderPassMetrics) {
        std::cout << pass.name << ":\n"
                  << "  GPU Time: " << std::fixed << std::setprecision(2) << pass.gpuTime << " ms\n"
                  << "  Vertices: " << pass.vertices << "\n"
                  << "  Triangles: " << pass.triangles << "\n";
    }
}

void PerformanceMonitor::logGPUMetrics() const {
    const auto& metrics = gpuMonitor.getMetrics();
    
    std::cout << "\nGPU Metrics:\n"
              << "------------\n"
              << "Vendor: " << metrics.vendor << "\n"
              << "GPU: " << metrics.gpuName << "\n";
    
    if (metrics.utilizationPercent >= 0) {
        std::cout << "Utilization: " << std::fixed << std::setprecision(1) 
                  << metrics.utilizationPercent << "%\n";
    }
    
    if (metrics.memoryTotalBytes > 0) {
        std::cout << "Memory: " << (metrics.memoryUsedBytes / (1024 * 1024)) << " MB / "
                  << (metrics.memoryTotalBytes / (1024 * 1024)) << " MB\n";
    }
}

void PerformanceMonitor::updateMetrics() {
    // Update GPU metrics
    gpuMonitor.updateMetrics();
    
    // Get current metrics
    const auto& gpuMetrics = gpuMonitor.getMetrics();
    const auto& perfCounters = gpuMonitor.getPerformanceCounters();
    
    // Update current metrics
    currentMetrics.gpuTime = gpuMetrics.utilizationPercent >= 0 ? gpuMetrics.utilizationPercent : 0.0;
    currentMetrics.memoryUsage = getResidentMemory();
    currentMetrics.drawCalls = perfCounters.drawCalls;
    currentMetrics.stateChanges = perfCounters.stateChanges;
    currentMetrics.shaderSwitches = perfCounters.shaderSwitches;
    currentMetrics.textureBindings = perfCounters.textureBindings;
    currentMetrics.bufferBindings = perfCounters.bufferBindings;
}

void PerformanceMonitor::updateRenderPassMetrics() {
    renderPassMetrics.clear();
    
    // Get metrics for each render pass
    for (const auto& [passName, metrics] : gpuMonitor.getAllRenderPassMetrics()) {
        RenderPassMetrics passMetrics;
        passMetrics.name = passName;
        passMetrics.gpuTime = metrics.gpuTime;
        passMetrics.vertices = metrics.vertices;
        passMetrics.triangles = metrics.triangles;
        renderPassMetrics.push_back(passMetrics);
    }
}

size_t PerformanceMonitor::getResidentMemory() const {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert from KB to bytes
    }
    return 0;
} 