#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

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
}

void PerformanceMonitor::endRenderPass(const std::string& passName) {
    if (passName == "Render") {
        auto renderEnd = std::chrono::high_resolution_clock::now();
        double cpuTime = std::chrono::duration_cast<std::chrono::microseconds>(renderEnd - renderStart).count() / 1000.0;
        currentMetrics.cpuTime = cpuTime;
    }
}

void PerformanceMonitor::trackDrawCall(size_t vertices, size_t triangles) {
    currentMetrics.drawCalls++;
}

void PerformanceMonitor::trackStateChange() {
    currentMetrics.stateChanges++;
}

void PerformanceMonitor::trackShaderSwitch() {
    currentMetrics.shaderSwitches++;
}

void PerformanceMonitor::trackTextureBinding() {
    currentMetrics.textureBindings++;
}

void PerformanceMonitor::trackBufferBinding() {
    currentMetrics.bufferBindings++;
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

void PerformanceMonitor::logMetrics() const {
    const auto& metrics = getLatestMetrics();
    
    std::cout << "\nPerformance Metrics:\n"
              << "-------------------\n"
              << "Frame Time: " << std::fixed << std::setprecision(2) << metrics.frameTime << " ms\n"
              << "FPS: " << std::fixed << std::setprecision(1) << metrics.fps << "\n"
              << "CPU Time: " << metrics.cpuTime << " ms\n"
              << "Memory Usage: " << (metrics.memoryUsage / (1024 * 1024)) << " MB\n"
              << "Draw Calls: " << metrics.drawCalls << "\n"
              << "State Changes: " << metrics.stateChanges << "\n"
              << "Shader Switches: " << metrics.shaderSwitches << "\n"
              << "Texture Bindings: " << metrics.textureBindings << "\n"
              << "Buffer Bindings: " << metrics.bufferBindings << "\n";
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

void PerformanceMonitor::updateMetrics() {
    currentMetrics.memoryUsage = getResidentMemory();
}

void PerformanceMonitor::updateRenderPassMetrics() {
    renderPassMetrics.clear();
}

size_t PerformanceMonitor::getResidentMemory() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert from KB to bytes
    }
    return 0;
#endif
} 