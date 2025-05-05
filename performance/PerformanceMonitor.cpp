#include "PerformanceMonitor.h"
#include "GPUMonitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <GL/glew.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

PerformanceMonitor::PerformanceMonitor(size_t historySize)
    : historySize(historySize)
    , lastFrameTime(0.0)
    , frameStartTime(0.0)
    , frameEndTime(0.0)
    , fps(0.0f)
    , frameTime(0.0f)
    , renderStartTime(0.0)
    , renderEndTime(0.0)
    , renderTime(0.0f)
{
    // Initialize GPU monitor
    gpuMonitor = std::make_unique<GPUMonitor>();
    if (gpuMonitor->isAvailable()) {
        std::cout << "GPU monitoring initialized successfully" << std::endl;
    } else {
        std::cout << "GPU monitoring not available" << std::endl;
    }
    metricsHistory.resize(historySize);
}

PerformanceMonitor::~PerformanceMonitor() {
    // Cleanup is handled by member destructors
}

void PerformanceMonitor::beginFrame() {
    frameStartTime = glfwGetTime();
}

void PerformanceMonitor::endFrame() {
    frameEndTime = glfwGetTime();
    frameTime = static_cast<float>((frameEndTime - frameStartTime) * 1000.0);
    fps = 1.0f / (frameTime / 1000.0f);
    
    // Update metrics
    updateMetrics();
    
    // Calculate frame metrics
    currentMetrics.frameTime = frameTime;
    currentMetrics.fps = fps;
    
    // Update history
    std::lock_guard<std::mutex> lock(metricsMutex);
    metricsHistory.push_back(currentMetrics);
    if (metricsHistory.size() > historySize) {
        metricsHistory.pop_front();
    }
}

void PerformanceMonitor::beginRenderPass(const std::string& passName) {
    if (passName == "Render") {
        renderStartTime = glfwGetTime();
    }
}

void PerformanceMonitor::endRenderPass(const std::string& passName) {
    if (passName == "Render") {
        renderEndTime = glfwGetTime();
        renderTime = static_cast<float>((renderEndTime - renderStartTime) * 1000.0);
        currentMetrics.cpuTime = renderTime;
        currentMetrics.gpuTime = renderTime;
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
    std::cout << "\nPerformance Metrics:" << std::endl;
    std::cout << "------------------" << std::endl;
    std::cout << "FPS: " << std::fixed << std::setprecision(1) << fps << std::endl;
    std::cout << "Frame Time: " << std::fixed << std::setprecision(2) << frameTime << " ms" << std::endl;
    std::cout << "CPU Time: " << std::fixed << std::setprecision(2) << currentMetrics.cpuTime << " ms" << std::endl;
    std::cout << "GPU Time: " << std::fixed << std::setprecision(2) << currentMetrics.gpuTime << " ms" << std::endl;
    std::cout << "Memory Usage: " << formatMemorySize(currentMetrics.memoryUsage) << std::endl;
    std::cout << "Draw Calls: " << currentMetrics.drawCalls << std::endl;
    std::cout << "State Changes: " << currentMetrics.stateChanges << std::endl;
    std::cout << "Shader Switches: " << currentMetrics.shaderSwitches << std::endl;
    std::cout << "Texture Bindings: " << currentMetrics.textureBindings << std::endl;
    std::cout << "Buffer Bindings: " << currentMetrics.bufferBindings << std::endl;

    // Log GPU metrics if available
    if (gpuMonitor && gpuMonitor->isAvailable()) {
        const auto& gpuMetrics = currentMetrics.gpuMetrics;
        std::cout << "\nGPU Metrics:" << std::endl;
        std::cout << "-----------" << std::endl;
        std::cout << "Vendor: " << gpuMetrics.vendor << std::endl;
        std::cout << "GPU: " << gpuMetrics.gpuName << std::endl;
        
        // Only show memory info if available
        if (gpuMetrics.memoryTotalBytes > 0) {
            std::cout << "Memory Used: " << formatMemorySize(gpuMetrics.memoryUsedBytes) << std::endl;
            std::cout << "Total Memory: " << formatMemorySize(gpuMetrics.memoryTotalBytes) << std::endl;
        }

        // Only show utilization if available
        if (gpuMetrics.utilizationPercent >= 0.0f) {
            std::cout << "GPU Utilization: " << std::fixed << std::setprecision(1) 
                     << gpuMetrics.utilizationPercent << "%" << std::endl;
        }

        // Only show temperature if available
        if (gpuMetrics.temperature > 0.0f) {
            std::cout << "Temperature: " << std::fixed << std::setprecision(1) 
                     << gpuMetrics.temperature << "°C" << std::endl;
        }

        // Only show power usage if available
        if (gpuMetrics.powerUsage > 0.0f) {
            std::cout << "Power Usage: " << std::fixed << std::setprecision(1) 
                     << gpuMetrics.powerUsage << "W" << std::endl;
        }
    } else {
        std::cout << "\nGPU Metrics: Not available" << std::endl;
    }
}

void PerformanceMonitor::updateMetrics() {
    // Update frame timing
    frameEndTime = glfwGetTime();
    frameTime = static_cast<float>((frameEndTime - frameStartTime) * 1000.0);
    fps = 1.0f / (frameTime / 1000.0f);

    // Update CPU timing
    currentMetrics.cpuTime = frameTime - renderTime;
    currentMetrics.gpuTime = renderTime;
    currentMetrics.frameTime = frameTime;
    currentMetrics.fps = fps;

    // Update GPU metrics if available
    if (gpuMonitor && gpuMonitor->isAvailable()) {
        if (gpuMonitor->updateMetrics()) {
            currentMetrics.gpuMetrics = gpuMonitor->getMetrics();
        }
    }

    // Update memory metrics
    updateMemoryMetrics();

    // Update history
    metricsHistory.push_back(currentMetrics);
    if (metricsHistory.size() > historySize) {
        metricsHistory.pop_front();
    }

    // Update render pass metrics
    updateRenderPassMetrics();
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

std::string PerformanceMonitor::formatMemorySize(size_t bytes) const {
    if (bytes < 1024) {
        return std::to_string(bytes) + " bytes";
    } else if (bytes < 1024 * 1024) {
        return std::to_string(bytes / 1024) + " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    } else {
        return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
    }
}

void PerformanceMonitor::updateMemoryMetrics() {
    currentMetrics.memoryUsage = getResidentMemory();
} 