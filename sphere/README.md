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
- Automatic destruction of stationary spheres (velocity < 0.1 units/s)
- Dynamic sphere spawning with 0.5s cooldown
- Realistic bounce behavior with energy loss
- Collision logging and statistics tracking

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
- `stationaryThreshold`: Velocity threshold for considering a sphere stationary (default: 0.1)
- `spawnCooldown`: Time between sphere spawns (default: 0.5s)
- `initialVelocity`: Starting velocity for new spheres (default: -2.0 units/s)

### Mesh Quality
- `sectors`: Horizontal mesh divisions (default: 16)
- `stacks`: Vertical mesh divisions (default: 16)
- `radius`: Sphere radius (default: 0.1 units)

### Spawn Settings
- `spawnHeight`: Initial Y position (default: 3.0-6.0)
- `spawnRange`: Initial X/Z position range (default: ±3.0)
- `spawnCooldown`: Time between spawns (default: 0.5s)
- `colorRange`: Random color generation range (default: 0.5-1.0)

## Dependencies
- GLM (OpenGL Mathematics)
- Modern C++ compiler (C++17 or later)
- OpenGL 4.5+ support

## Performance Considerations

### Memory Usage
- Single shared mesh for all spheres
- Minimal per-sphere memory footprint
- Efficient instance rendering
- Automatic cleanup of stationary spheres
- Optimized spawn cooldown system

### CPU Usage
- Independent physics calculations
- Optimized collision detection
- Scalable with sphere count
- Efficient stationary sphere detection
- Minimal spawn overhead

### GPU Usage
- Shared vertex/index buffers
- Efficient instanced rendering
- Minimal state changes
- Optimized color updates
- Efficient transform updates

## Future Improvements
1. Sphere-to-sphere collisions
2. Spatial partitioning for collision detection
3. Parallelized physics calculations
4. Advanced material properties
5. Texture support
6. Custom physics parameters per sphere
7. Configurable stationary thresholds
8. Advanced spawn patterns

## Contributing
When contributing to this module:
1. Follow existing code style
2. Document new features
3. Update this README for significant changes
4. Add unit tests for new functionality
5. Consider performance implications 