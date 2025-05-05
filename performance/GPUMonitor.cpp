#include "GPUMonitor.h"
#include <GL/glew.h>
#include <dlfcn.h>
#include <iostream>

// NVIDIA GPU memory info extension
#ifndef GL_NVX_gpu_memory_info
#define GL_NVX_gpu_memory_info
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX           0x904B
#endif

// AMD memory info extension
#ifndef GL_ATI_meminfo
#define GL_ATI_meminfo
#define GL_VBO_FREE_MEMORY_ATI                     0x87FB
#define GL_TEXTURE_FREE_MEMORY_ATI                 0x87FC
#define GL_RENDERBUFFER_FREE_MEMORY_ATI            0x87FD
#endif

// Performance monitoring extensions
#ifndef GL_ARB_timer_query
#define GL_ARB_timer_query
#define GL_TIME_ELAPSED 0x88BF
#define GL_TIMESTAMP 0x8E28
#endif

GPUMonitor::GPUMonitor()
    : available(false)
    , driverHandle(nullptr)
{
    // Initialize performance counters
    resetPerformanceCounters();

    // Try NVIDIA first, then AMD
    if (!initNVIDIA()) {
        if (!initAMD()) {
            // Fallback to OpenGL for basic info
            const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            if (vendor && renderer) {
                metrics.vendor = vendor;
                metrics.gpuName = renderer;
                available = true;
            }
        }
    }

    // Initialize performance monitoring
    initPerformanceMonitoring();

    // Print timer query support status
    if (glewIsSupported("GL_ARB_timer_query")) {
        std::cout << "GL_ARB_timer_query is supported. GPU timing is available." << std::endl;
    } else {
        std::cout << "GL_ARB_timer_query is NOT supported. GPU timing is NOT available." << std::endl;
    }
}

GPUMonitor::~GPUMonitor() {
    // Clean up timer queries
    for (const auto& query : renderPassQueries) {
        glDeleteQueries(1, &query.second);
    }
    cleanup();
}

void GPUMonitor::initPerformanceMonitoring() {
    // Check for timer query support
    if (glewIsSupported("GL_ARB_timer_query")) {
        GLint queryBits = 0;
        glGetQueryiv(GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS, &queryBits);
        if (queryBits > 0) {
            available = true;
        }
    }
}

void GPUMonitor::beginRenderPass(const std::string& passName) {
    if (!available) return;
    currentPass = passName;
    // Create timer query if it doesn't exist
    if (renderPassQueries.find(passName) == renderPassQueries.end()) {
        GLuint query = 0;
        glGenQueries(1, &query);
        if (query == 0) return; // Failed to create query
        renderPassQueries[passName] = query;
        renderPassMetrics[passName] = RenderPassMetrics{};
    }

    // Start timing
    GLuint query = renderPassQueries[passName];
    if (query != 0)
        glBeginQuery(GL_TIME_ELAPSED, query);
}

void GPUMonitor::endRenderPass(const std::string& passName) {
    if (!available) return;
    auto it = renderPassQueries.find(passName);
    if (it == renderPassQueries.end() || it->second == 0) return;

    glEndQuery(GL_TIME_ELAPSED);

    GLint resultAvailable = 0;
    glGetQueryObjectiv(it->second, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);
    if (resultAvailable) {
        GLuint64 timeElapsed;
        glGetQueryObjectui64v(it->second, GL_QUERY_RESULT, &timeElapsed);
        renderPassMetrics[passName].gpuTime = timeElapsed / 1000000.0f; // ms
    }
    currentPass.clear();
}

const GPUMonitor::RenderPassMetrics& GPUMonitor::getRenderPassMetrics(const std::string& passName) const {
    static RenderPassMetrics emptyMetrics;
    auto it = renderPassMetrics.find(passName);
    return it != renderPassMetrics.end() ? it->second : emptyMetrics;
}

void GPUMonitor::resetPerformanceCounters() {
    performanceCounters = PerformanceCounters{};
}

void GPUMonitor::trackDrawCall(size_t vertices, size_t triangles) {
    performanceCounters.drawCalls++;
    renderPassMetrics[currentPass].vertices += vertices;
    renderPassMetrics[currentPass].triangles += triangles;
}

void GPUMonitor::trackStateChange() {
    performanceCounters.stateChanges++;
}

void GPUMonitor::trackShaderSwitch() {
    performanceCounters.shaderSwitches++;
}

void GPUMonitor::trackTextureBinding() {
    performanceCounters.textureBindings++;
}

void GPUMonitor::trackBufferBinding() {
    performanceCounters.bufferBindings++;
}

bool GPUMonitor::updateMetrics() {
    if (!available) {
        return false;
    }

    // Get basic GPU metrics using OpenGL
    GLint totalMemoryKB = 0;
    GLint currentMemoryKB = 0;
    
    // Try to get memory info using GL_NVX_gpu_memory_info for NVIDIA
    if (metrics.vendor == "NVIDIA") {
        glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKB);
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKB);
        
        metrics.memoryTotalBytes = static_cast<size_t>(totalMemoryKB) * 1024;
        metrics.memoryUsedBytes = static_cast<size_t>(totalMemoryKB - currentMemoryKB) * 1024;
    }
    // Try to get memory info using GL_ATI_meminfo for AMD
    else if (metrics.vendor == "AMD") {
        GLint info[4];
        glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, info);
        
        totalMemoryKB = info[0];  // Total memory available
        currentMemoryKB = info[1]; // Largest available block
        
        metrics.memoryTotalBytes = static_cast<size_t>(totalMemoryKB) * 1024;
        metrics.memoryUsedBytes = static_cast<size_t>(totalMemoryKB - currentMemoryKB) * 1024;
    }

    // Update performance counters
    updatePerformanceCounters();

    return true;
}

void GPUMonitor::updatePerformanceCounters() {
    // Update render pass metrics
    for (auto& [passName, metrics] : renderPassMetrics) {
        auto it = renderPassQueries.find(passName);
        if (it == renderPassQueries.end() || it->second == 0) continue;
        GLint available = 0;
        glGetQueryObjectiv(it->second, GL_QUERY_RESULT_AVAILABLE, &available);
        if (available) {
            GLuint64 timeElapsed;
            glGetQueryObjectui64v(it->second, GL_QUERY_RESULT, &timeElapsed);
            metrics.gpuTime = timeElapsed / 1000000.0f; // Convert to milliseconds
        }
    }
}

bool GPUMonitor::initNVIDIA() {
    // Try to load NVIDIA Management Library dynamically
    void* nvml = dlopen("libnvidia-ml.so", RTLD_NOW);
    if (!nvml) {
        return false;
    }

    // Store handle for cleanup
    driverHandle = nvml;
    available = true;
    metrics.vendor = "NVIDIA";

    // Get GPU name using OpenGL (more reliable than NVML in some cases)
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    if (renderer) {
        metrics.gpuName = renderer;
    }

    return true;
}

bool GPUMonitor::initAMD() {
    // Try to load AMD Display Library dynamically
    void* adl = dlopen("libatiadlxx.so", RTLD_NOW);
    if (!adl) {
        return false;
    }

    // Store handle for cleanup
    driverHandle = adl;
    available = true;
    metrics.vendor = "AMD";

    // Get GPU name using OpenGL (more reliable than ADL in some cases)
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    if (renderer) {
        metrics.gpuName = renderer;
    }

    return true;
}

void GPUMonitor::cleanup() {
    if (driverHandle) {
        dlclose(driverHandle);
        driverHandle = nullptr;
    }
} 