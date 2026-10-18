# Hello Triangle
Assignment — Hello Triangle

A university 3D programming assignment, focused on rendering a textured 3D quad implemented using Direct3D 11 and C++ .
Overview

The goal of this project is to develop a simple 3D application in C++ using Direct3D 11 (D3D11). The application renders a quad constructed from two triangles, applies a texture, and animates the quad by rotating it around the Y-axis with an offset from the origin.

The project also implements the Phong reflection model to simulate lighting, including ambient, diffuse, and specular components, using HLSL shaders.
Features

    3D rendering: A quad constructed from two triangles.
    Texture mapping: UV coordinates used to apply a 2D texture to the quad.
    3D transformations: World, view, and perspective projection matrices.
    Animated rotation: Continuous Y-axis rotation around an offset point.
    Phong lighting: Per-pixel lighting calculations in the pixel shader.
    Constant buffers: Data transferred to shaders for transformations, lighting, and camera information.
    Direct3D 11: GPU-accelerated rendering using C++ and HLSL.

Technologies

    C++
    Direct3D 11
    HLSL (High-Level Shading Language)
    DirectXMath
    Visual Studio

Getting Started
Requirements

    Windows with Direct3D 11 support
    Visual Studio 2017 or 2019, or a compatible development environment
    A compatible Windows SDK
    The project’s texture assets and shader files

Build and Run

    Clone or download this repository.
    Open the solution in Visual Studio.
    Make sure the Windows SDK and project dependencies are configured.
    Build the project.
    Run the application to view the textured, rotating quad and its lighting effects.

Learning Objectives

This assignment explores the fundamentals of real-time 3D graphics programming, including:

    Vertex data and triangle-based geometry
    Texture coordinates and surface normals
    World, view, and projection transformations
    Time-based animation
    Vertex and pixel shaders
    The Phong reflection model
    Direct3D resource management
