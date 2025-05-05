#include "GPUMonitor.h"
#include <GL/glew.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
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
            const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            
            if (vendor && renderer && version) {
                metrics.vendor = vendor;
                metrics.gpuName = renderer;
                available = true;
                std::cout << "GPU Info:" << std::endl;
                std::cout << "  Vendor: " << vendor << std::endl;
                std::cout << "  Renderer: " << renderer << std::endl;
                std::cout << "  Version: " << version << std::endl;

                // Check for GPU memory info extensions
                if (metrics.vendor.find("Intel") != std::string::npos) {
                    std::cout << "  Intel GPU detected" << std::endl;
                    
                    // Check for timer query support
                    if (glewIsSupported("GL_ARB_timer_query")) {
                        std::cout << "  GL_ARB_timer_query supported - GPU timing available" << std::endl;
                        GLint queryBits;
                        glGetQueryiv(GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS, &queryBits);
                        std::cout << "  Timer query bits: " << queryBits << std::endl;
                    }
                    
                    // Try to get initial memory info
                    GLint totalMemoryMB = 0;
                    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryMB);
                    if (totalMemoryMB > 0) {
                        std::cout << "  Total GPU memory: " << totalMemoryMB << " MB" << std::endl;
                        metrics.memoryTotalBytes = static_cast<size_t>(totalMemoryMB) * 1024 * 1024;
                    } else {
                        std::cout << "  Using default memory size for Intel UHD 630: 1GB" << std::endl;
                        metrics.memoryTotalBytes = 1073741824; // 1GB
                    }
                    
                    // Initialize metrics with default values
                    metrics.utilizationPercent = 0.0f;
                    metrics.memoryUsedBytes = 0;
                    metrics.temperature = 0.0f;
                    metrics.powerUsage = 0.0f;
                }
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
    if (!currentPass.empty()) {
        renderPassMetrics[currentPass].vertices += vertices;
        renderPassMetrics[currentPass].triangles += triangles;
    }
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
    if (metrics.vendor.find("NVIDIA") != std::string::npos) {
        if (glewIsSupported("GL_NVX_gpu_memory_info")) {
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKB);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKB);
            
            metrics.memoryTotalBytes = static_cast<size_t>(totalMemoryKB) * 1024;
            metrics.memoryUsedBytes = static_cast<size_t>(totalMemoryKB - currentMemoryKB) * 1024;
        }
    }
    // Try to get memory info using GL_ATI_meminfo for AMD
    else if (metrics.vendor.find("AMD") != std::string::npos) {
        if (glewIsSupported("GL_ATI_meminfo")) {
            GLint info[4];
            glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, info);
            
            totalMemoryKB = info[0];  // Total memory available
            currentMemoryKB = info[1]; // Largest available block
            
            metrics.memoryTotalBytes = static_cast<size_t>(totalMemoryKB) * 1024;
            metrics.memoryUsedBytes = static_cast<size_t>(totalMemoryKB - currentMemoryKB) * 1024;
        }
    }
    // Handle Intel GPU metrics
    else if (metrics.vendor.find("Intel") != std::string::npos) {
        // For Intel GPUs, we'll use basic OpenGL info and timer queries
        // Set default memory size for Intel UHD 630
        metrics.memoryTotalBytes = 1073741824; // 1GB in bytes
        
        // Get GPU utilization through GL timer queries
        if (glewIsSupported("GL_ARB_timer_query")) {
            GLuint query;
            glGenQueries(1, &query);
            
            // Start timing
            glBeginQuery(GL_TIME_ELAPSED, query);
            // Perform a small GPU operation
            glFinish();
            glEndQuery(GL_TIME_ELAPSED);
            
            // Wait for the result
            GLint available = 0;
            while (!available) {
                glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
            }
            
            // Get the GPU time
            GLuint64 gpuTime;
            glGetQueryObjectui64v(query, GL_QUERY_RESULT, &gpuTime);
            
            // Calculate rough GPU utilization based on time spent
            double timeMs = double(gpuTime) / 1000000.0;
            float utilization = float(timeMs * 100.0);
            metrics.utilizationPercent = utilization > 100.0f ? 100.0f : utilization;
            
            glDeleteQueries(1, &query);
        }

        // For Intel GPUs, we can't get accurate memory usage through OpenGL
        // So we'll estimate based on the total memory
        metrics.memoryUsedBytes = metrics.memoryTotalBytes / 2; // Rough estimate
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
#ifdef _WIN32
    // Try to load NVIDIA Management Library
    driverHandle = LoadLibraryA("nvml.dll");
    if (!driverHandle) {
        return false;
    }

    // Get function pointers
    auto nvmlInit = (NVML_INIT_FUNC)GetProcAddress((HMODULE)driverHandle, "nvmlInit_v2");
    auto nvmlDeviceGetHandleByIndex = (NVML_DEVICE_GET_HANDLE_FUNC)GetProcAddress((HMODULE)driverHandle, "nvmlDeviceGetHandleByIndex");
    auto nvmlDeviceGetMemoryInfo = (NVML_DEVICE_GET_MEMORY_INFO_FUNC)GetProcAddress((HMODULE)driverHandle, "nvmlDeviceGetMemoryInfo");
    auto nvmlDeviceGetUtilizationRates = (NVML_DEVICE_GET_UTILIZATION_RATES_FUNC)GetProcAddress((HMODULE)driverHandle, "nvmlDeviceGetUtilizationRates");
    auto nvmlDeviceGetName = (NVML_DEVICE_GET_NAME_FUNC)GetProcAddress((HMODULE)driverHandle, "nvmlDeviceGetName");

    if (!nvmlInit || !nvmlDeviceGetHandleByIndex || !nvmlDeviceGetMemoryInfo || !nvmlDeviceGetUtilizationRates || !nvmlDeviceGetName) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Initialize NVML
    if (nvmlInit() != NVML_SUCCESS) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get device handle
    nvmlDevice_t device;
    if (nvmlDeviceGetHandleByIndex(0, &device) != NVML_SUCCESS) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get device name
    char name[256];
    if (nvmlDeviceGetName(device, name, sizeof(name)) == NVML_SUCCESS) {
        metrics.gpuName = name;
    }

    metrics.vendor = "NVIDIA";
    available = true;
    return true;
#else
    // Try to load NVIDIA Management Library dynamically
    driverHandle = dlopen("libnvidia-ml.so", RTLD_NOW);
    if (!driverHandle) {
        return false;
    }

    // Get function pointers
    auto nvmlInit = (NVML_INIT_FUNC)dlsym(driverHandle, "nvmlInit_v2");
    auto nvmlDeviceGetHandleByIndex = (NVML_DEVICE_GET_HANDLE_FUNC)dlsym(driverHandle, "nvmlDeviceGetHandleByIndex");
    auto nvmlDeviceGetMemoryInfo = (NVML_DEVICE_GET_MEMORY_INFO_FUNC)dlsym(driverHandle, "nvmlDeviceGetMemoryInfo");
    auto nvmlDeviceGetUtilizationRates = (NVML_DEVICE_GET_UTILIZATION_RATES_FUNC)dlsym(driverHandle, "nvmlDeviceGetUtilizationRates");
    auto nvmlDeviceGetName = (NVML_DEVICE_GET_NAME_FUNC)dlsym(driverHandle, "nvmlDeviceGetName");

    if (!nvmlInit || !nvmlDeviceGetHandleByIndex || !nvmlDeviceGetMemoryInfo || !nvmlDeviceGetUtilizationRates || !nvmlDeviceGetName) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Initialize NVML
    if (nvmlInit() != NVML_SUCCESS) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get device handle
    nvmlDevice_t device;
    if (nvmlDeviceGetHandleByIndex(0, &device) != NVML_SUCCESS) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get device name
    char name[256];
    if (nvmlDeviceGetName(device, name, sizeof(name)) == NVML_SUCCESS) {
        metrics.gpuName = name;
    }

    metrics.vendor = "NVIDIA";
    available = true;
    return true;
#endif
}

bool GPUMonitor::initAMD() {
#ifdef _WIN32
    // Try to load AMD ADL Library
    driverHandle = LoadLibraryA("atiadlxx.dll");
    if (!driverHandle) {
        return false;
    }

    // Get function pointers
    auto ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE_FUNC)GetProcAddress((HMODULE)driverHandle, "ADL_Main_Control_Create");
    auto ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET_FUNC)GetProcAddress((HMODULE)driverHandle, "ADL_Adapter_NumberOfAdapters_Get");
    auto ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET_FUNC)GetProcAddress((HMODULE)driverHandle, "ADL_Adapter_AdapterInfo_Get");
    auto ADL_Adapter_MemoryInfo_Get = (ADL_ADAPTER_MEMORYINFO_GET_FUNC)GetProcAddress((HMODULE)driverHandle, "ADL_Adapter_MemoryInfo_Get");

    if (!ADL_Main_Control_Create || !ADL_Adapter_NumberOfAdapters_Get || !ADL_Adapter_AdapterInfo_Get || !ADL_Adapter_MemoryInfo_Get) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Initialize ADL
    if (ADL_Main_Control_Create(nullptr, 1) != ADL_OK) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get number of adapters
    int numAdapters = 0;
    if (ADL_Adapter_NumberOfAdapters_Get(&numAdapters) != ADL_OK || numAdapters <= 0) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get adapter info
    std::vector<AdapterInfo> adapterInfo(numAdapters);
    if (ADL_Adapter_AdapterInfo_Get(adapterInfo.data(), sizeof(AdapterInfo) * numAdapters) != ADL_OK) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Find first AMD adapter
    for (const auto& adapter : adapterInfo) {
        if (adapter.iVendorID == 0x1002) { // AMD vendor ID
            metrics.gpuName = adapter.strAdapterName;
            metrics.vendor = "AMD";
            available = true;
            return true;
        }
    }

    FreeLibrary((HMODULE)driverHandle);
    driverHandle = nullptr;
    return false;
#else
    // Try to load AMD ADL Library
    driverHandle = dlopen("libatiadlxx.so", RTLD_NOW);
    if (!driverHandle) {
        return false;
    }

    // Get function pointers
    auto ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE_FUNC)dlsym(driverHandle, "ADL_Main_Control_Create");
    auto ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET_FUNC)dlsym(driverHandle, "ADL_Adapter_NumberOfAdapters_Get");
    auto ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET_FUNC)dlsym(driverHandle, "ADL_Adapter_AdapterInfo_Get");
    auto ADL_Adapter_MemoryInfo_Get = (ADL_ADAPTER_MEMORYINFO_GET_FUNC)dlsym(driverHandle, "ADL_Adapter_MemoryInfo_Get");

    if (!ADL_Main_Control_Create || !ADL_Adapter_NumberOfAdapters_Get || !ADL_Adapter_AdapterInfo_Get || !ADL_Adapter_MemoryInfo_Get) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Initialize ADL
    if (ADL_Main_Control_Create(nullptr, 1) != ADL_OK) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get number of adapters
    int numAdapters = 0;
    if (ADL_Adapter_NumberOfAdapters_Get(&numAdapters) != ADL_OK || numAdapters <= 0) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Get adapter info
    std::vector<AdapterInfo> adapterInfo(numAdapters);
    if (ADL_Adapter_AdapterInfo_Get(adapterInfo.data(), sizeof(AdapterInfo) * numAdapters) != ADL_OK) {
        dlclose(driverHandle);
        driverHandle = nullptr;
        return false;
    }

    // Find first AMD adapter
    for (const auto& adapter : adapterInfo) {
        if (adapter.iVendorID == 0x1002) { // AMD vendor ID
            metrics.gpuName = adapter.strAdapterName;
            metrics.vendor = "AMD";
            available = true;
            return true;
        }
    }

    dlclose(driverHandle);
    driverHandle = nullptr;
    return false;
#endif
}

void GPUMonitor::cleanup() {
#ifdef _WIN32
    if (driverHandle) {
        FreeLibrary((HMODULE)driverHandle);
        driverHandle = nullptr;
    }
#else
    if (driverHandle) {
        dlclose(driverHandle);
        driverHandle = nullptr;
    }
#endif
} 