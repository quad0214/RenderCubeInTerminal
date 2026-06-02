# RenderCubeInTerminal

A **software rasterizer implementation** that renders a 3D rotating cube in Windows terminal/command prompt. This project demonstrates a complete rendering pipeline from vertex transformation through rasterization using fixed-point and floating-point arithmetic.

![image](https://user-images.githubusercontent.com/34961209/214741189-964b30ac-ac83-4178-b2c0-2d7b42014f22.png)

## Features

### Complete Software Rasterizer Pipeline
- **Vertex Transformation**: Full 3D transformation pipeline with rotation matrices
- **Homogeneous Clipping**: Sutherland-Hodgman polygon clipping against view frustum planes
- **Perspective Division**: Conversion from homogeneous coordinates to screen space
- **Back-Face Culling**: Efficient triangle culling for hidden surface removal
- **Viewport Transformation**: Screen-space coordinate mapping

### Dual Arithmetic Implementations
- **Fixed-Point Rasterization**: Precision-controlled fixed-point arithmetic with configurable precision levels
- **Floating-Point Rasterization**: Standard IEEE 754 floating-point implementation with dynamic rasterization modes

### Optimizations
- **Hierarchical Rasterization**: Multi-level tile-based processing for efficient triangle coverage
- **Memory Pooling**: Allocation-free rasterization with pre-allocated memory pools
- **Edge Function Precomputation**: Optimization of edge derivatives for tile-based processing
- **Bounding Box Culling**: Viewport clipping before triangle rasterization

### Pixel Rendering
- **Pluggable Pixel Shader System**: Extensible interface for custom pixel processing
- **Depth Testing**: Z-buffer implementation for proper occlusion handling
- **ASCII Rendering**: Terminal output with depth-based character selection

## Overview

The rasterizer implements a complete graphics pipeline:

```
Input Vertices → Transform → Clip → Perspective Divide → 
Cull → Viewport Transform → Rasterize → Output Pixels
```

**Clipping**: Uses Sutherland-Hodgman algorithm to clip triangles against 6 frustum planes, supporting polygon expansion up to 9 triangles per input.

**Rasterization**: Implements edge function method for point-in-triangle testing with both fixed-point and floating-point backends, including depth interpolation for proper surface visibility.

## Build

### Requirements
- Windows 10/11
- Visual Studio 2019 or later with C++17 support

### Build Instructions
```bash
git clone https://github.com/quad0214/RenderCubeInTerminal.git
# Open RenderCubeInTerminal.sln in Visual Studio and build
```

## Inspiration

Inspired by [this video](https://www.youtube.com/embed/p09i_hoFdd0) on approximate rasterization techniques.

## Contributing

Found a bug or have suggestions? Please open an [issue](https://github.com/quad0214/RenderCubeInTerminal/issues).
