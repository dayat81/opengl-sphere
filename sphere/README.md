# Sphere Physics Module

## Overview
This module implements a real-time 3D sphere physics simulation system using modern OpenGL. It provides a complete solution for rendering and simulating multiple spheres with realistic physics behavior.

## Architecture

### Core Components

#### `SphereHandler` (SphereHandler.h/cpp)
- Main facade for the sphere subsystem
- Coordinates physics, spawning, and rendering
- Manages sphere lifecycle and shared resources
- Provides simple interface for main application

#### `Sphere` (Sphere.h/cpp)
- Individual sphere entity
- Handles physics calculations
- Manages position, velocity, and collisions
- Tracks collision statistics

#### `SphereMesh` (SphereMesh.h/cpp)
- Generates and manages 3D sphere geometry
- UV sphere topology with configurable resolution
- Shared mesh data for efficient rendering
- Vertex and index buffer management

#### `SphereSpawner` (SphereSpawner.h/cpp)
- Controls sphere creation timing
- Manages spawn positions and intervals
- Provides randomization for initial conditions
- Configurable spawn parameters

## Features

### Physics Simulation
- Gravity-based motion
- Elastic collisions with boundaries
- Energy loss through bounce factor
- Semi-implicit Euler integration

### Rendering
- Modern OpenGL (4.5+) rendering
- Efficient shared mesh data
- Dynamic instance rendering
- Configurable visual properties

### Performance
- Optimized mesh sharing
- Efficient collision detection
- Scalable for multiple objects
- Performance monitoring support

## Usage Example

```cpp
// Create sphere handler with default radius
SphereHandler sphereHandler(0.1f);

// Main update loop
while (running) {
    float deltaTime = calculateDeltaTime();
    
    // Update physics and spawning
    sphereHandler.update(deltaTime);
    
    // Render all spheres
    for (const auto& sphere : sphereHandler.getSpheres()) {
        // Get sphere properties
        auto position = sphere.getPosition();
        auto color = sphere.getColor();
        
        // Render sphere...
    }
}
```

## Configuration

### Physics Parameters
- `gravity`: Acceleration due to gravity (default: 9.81 units/s²)
- `bounceFactor`: Energy retention in collisions (default: 0.7)
- `floorY`: Floor plane Y-coordinate (default: -1.0)

### Mesh Quality
- `sectors`: Horizontal mesh divisions (default: 16)
- `stacks`: Vertical mesh divisions (default: 16)

### Spawn Settings
- `spawnHeight`: Initial Y position (default: 2.0)
- `spawnInterval`: Time between spawns (default: 1.0s)

## Dependencies
- GLM (OpenGL Mathematics)
- Modern C++ compiler (C++17 or later)
- OpenGL 4.5+ support

## Performance Considerations

### Memory Usage
- Single shared mesh for all spheres
- Minimal per-sphere memory footprint
- Efficient instance rendering

### CPU Usage
- Independent physics calculations
- Optimized collision detection
- Scalable with sphere count

### GPU Usage
- Shared vertex/index buffers
- Efficient instanced rendering
- Minimal state changes

## Future Improvements
1. Sphere-to-sphere collisions
2. Spatial partitioning for collision detection
3. Parallelized physics calculations
4. Advanced material properties
5. Texture support
6. Custom physics parameters per sphere

## Contributing
When contributing to this module:
1. Follow existing code style
2. Document new features
3. Update this README for significant changes
4. Add unit tests for new functionality
5. Consider performance implications 