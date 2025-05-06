# OpenGL Physics Simulation

## Overview
A modern OpenGL application demonstrating real-time physics simulation, 3D rendering, and performance monitoring. The project showcases modern C++ design patterns, efficient graphics programming, and modular architecture.

![Physics Simulation](docs/simulation.gif)

## Features

### Real-time Physics
- Gravity-based motion simulation
- Elastic collision handling
- Configurable physics parameters
- Smooth interpolation
- Automatic destruction of stationary spheres (velocity < 0.1 units/s)
- Dynamic sphere spawning with 0.5s cooldown
- Realistic bounce behavior with energy loss

### 3D Graphics
- Modern OpenGL 4.5+ rendering
- Efficient mesh management
- Dynamic object spawning
- Configurable visual properties

### Performance Monitoring
- Real-time FPS tracking
- Frame time analysis
- Memory usage monitoring
- Physics statistics
- Collision counting

## Project Structure

```
OpenGL-Example/
├── sphere/              # Sphere physics module
│   ├── README.md       # Module documentation
│   ├── Sphere.h/cpp    # Individual sphere logic
│   ├── SphereMesh.h/cpp    # Mesh generation
│   ├── SphereSpawner.h/cpp # Object spawning
│   └── SphereHandler.h/cpp # Main module interface
├── Util.h              # OpenGL utilities
├── Debug.h             # Debug helpers
├── PerformanceMonitor.h/cpp # Performance tracking
└── main.cpp            # Application entry point
```

## Building the Project

### Prerequisites
- C++17 compatible compiler
- OpenGL 4.5+ support
- GLEW
- GLFW3
- GLM

### Linux/WSL
```bash
# Install dependencies
sudo apt-get install libglew-dev libglfw3-dev libglm-dev

# Build
mkdir build && cd build
g++ ../main.cpp ../sphere/*.cpp ../PerformanceMonitor.cpp -o main -lglfw -lGLEW -lGL -I/usr/include/glm -I..
```

### Windows (MSVC)
```batch
# Using Visual Studio
- Open project in Visual Studio
- Set include directories for GLEW, GLFW, and GLM
- Build solution

# Using Command Line
cl /EHsc /std:c++17 main.cpp sphere/*.cpp PerformanceMonitor.cpp /I"include" /link glfw3.lib glew32.lib opengl32.lib
```

## Usage

### Running the Application
```bash
./main
```

### Controls
- ESC: Exit application
- Space: Pause/Resume simulation
- R: Reset simulation
- +/-: Add new sphere (0.5s cooldown)

### Configuration
Key parameters can be adjusted in the code:
```cpp
// Physics settings
constexpr float gravity = 9.81f;
constexpr float bounceFactor = 0.7f;
constexpr float stationaryThreshold = 0.1f;  // Velocity threshold for stationary spheres
constexpr float spawnCooldown = 0.5f;       // Time between sphere spawns

// Rendering settings
constexpr uint32_t width = 720;
constexpr uint32_t height = 480;

// Performance settings
constexpr size_t historySize = 100;
```

## Performance

### Optimization Techniques
1. Shared mesh data for all spheres
2. Efficient collision detection
3. Modern OpenGL best practices
4. Memory pool for dynamic objects
5. Automatic cleanup of stationary spheres
6. Optimized spawn cooldown system

### Benchmarks
Tested on reference system (Intel i7, NVIDIA RTX 3070):
- 100 spheres: 400+ FPS
- 500 spheres: 200+ FPS
- 1000 spheres: 100+ FPS

Note: Performance may vary based on:
- Number of active spheres
- Collision frequency
- Spawn rate
- System specifications

## Modules

### Sphere Physics (`/sphere`)
Complete physics simulation system. See [sphere/README.md](sphere/README.md) for details.

### Performance Monitoring
Real-time performance tracking and analysis:
- Frame timing
- Memory usage
- Object counts
- Physics statistics

### OpenGL Utilities
Helper functions for:
- Context creation
- Shader management
- Buffer handling
- Error checking

## Contributing

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Set up development environment
4. Make changes
5. Submit pull request

### Coding Standards
- Follow existing code style
- Document public interfaces
- Add unit tests for new features
- Update relevant documentation

### Testing
- Run performance benchmarks
- Test on different GPU vendors
- Verify memory usage
- Check error conditions

## License
MIT License - See LICENSE file for details

## Acknowledgments
- OpenGL Mathematics (GLM)
- GLFW team
- GLEW maintainers
- OpenGL community

## Contact
For questions or suggestions:
- Open an issue
- Submit a pull request
- Contact maintainers 