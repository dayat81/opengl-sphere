#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include <unordered_map>
#include <memory>

// NVIDIA Management Library types
typedef void* nvmlDevice_t;
typedef int nvmlReturn_t;
#define NVML_SUCCESS 0

// AMD Display Library types
typedef int ADL_STATUS;
#define ADL_OK 0
typedef void* ADL_MAIN_MALLOC_CALLBACK;
typedef void* ADL_MAIN_FREE_CALLBACK;

// Function pointer types for NVIDIA
typedef nvmlReturn_t (*NVML_INIT_FUNC)();
typedef nvmlReturn_t (*NVML_DEVICE_GET_HANDLE_FUNC)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*NVML_DEVICE_GET_MEMORY_INFO_FUNC)(nvmlDevice_t, void*);
typedef nvmlReturn_t (*NVML_DEVICE_GET_UTILIZATION_RATES_FUNC)(nvmlDevice_t, void*);
typedef nvmlReturn_t (*NVML_DEVICE_GET_NAME_FUNC)(nvmlDevice_t, char*, unsigned int);

// Function pointer types for AMD
typedef ADL_STATUS (*ADL_MAIN_CONTROL_CREATE_FUNC)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef ADL_STATUS (*ADL_ADAPTER_NUMBEROFADAPTERS_GET_FUNC)(int*);
typedef ADL_STATUS (*ADL_ADAPTER_ADAPTERINFO_GET_FUNC)(void*, int);
typedef ADL_STATUS (*ADL_ADAPTER_MEMORYINFO_GET_FUNC)(int, void*);

// AMD adapter info structure
struct AdapterInfo {
    int iSize;
    int iAdapterIndex;
    char strUDID[256];
    int iBusNumber;
    int iDeviceNumber;
    int iFunctionNumber;
    int iVendorID;
    char strAdapterName[256];
    char strDisplayName[256];
    int iPresent;
    int iExist;
    char strDriverPath[256];
    char strDriverPathExt[256];
    char strPNPString[256];
    int iOSDisplayIndex;
};

// Memory info structures
struct NVMLMemoryInfo {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
};

struct AMDMemoryInfo {
    int iMemorySize;
    int iMemoryBandwidth;
    int iMemoryType;
};

// GPU metrics structure
struct GPUMetrics {
    std::string vendor;
    std::string gpuName;
    float utilizationPercent = -1.0f;  // -1 indicates not available
    size_t memoryUsedBytes = 0;    // GPU memory used in bytes
    size_t memoryTotalBytes = 0;   // Total GPU memory in bytes
    float temperature = 0.0f;      // GPU temperature in Celsius
    float powerUsage = 0.0f;       // GPU power usage in watts
};

/**
 * @brief GPU monitoring system for collecting GPU metrics
 */
class GPUMonitor {
public:
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