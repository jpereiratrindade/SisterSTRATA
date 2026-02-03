# SisterSTRATA Execution Modes

This document defines the architectural boundaries for the multiple execution modes of the SisterSTRATA platform.

## 1. Core Principles

-   **Scientific Purity**: The Core Domain (`src/core`) MUST NOT depend on any visualization technology (Vulkan, SDL, ImGui). It is pure C++ logic.
-   **Observer Pattern**: Visualization is an *observer* of the scientific state, not the owner of it.
-   **Data Ownership**: The `Application` layer (via `Session`) owns the `WorldState`. The `Renderer` merely reads it to draw.

## 2. Modes of Execution

### Mode A: Graphical (Full)
-   **Target**: `SisterSTRATA` (default executable)
-   **Capabilities**: Interactive 3D, GUI, Real-time Simulation.
-   **Components**: Core + Application + Infrastructure + World3D (Vulkan).
-   **Use Case**: Research, Presentation, Data Exploration.

### Mode B: Headless (CLI)
-   **Target**: `SisterSTRATA_cli`
-   **Capabilities**: Batch Processing, Simulation Pipeline, Automated Reporting.
-   **Components**: Core + Application + Infrastructure (IO/LLM only).
-   **Excludes**: World3D, UI.
-   **Use Case**: Server processing, CI/CD pipelines, Large-scale analysis on non-GPU nodes.

## 3. Architecture Layers

1.  **Domain (Core)**: Entities (Soil, Hydro, Vegetation) and Value Objects (Points, Bounds).
2.  **Application**: Use Cases (Run Simulation, Load File) and State Management (`Session`, `WorldState`).
3.  **Infrastructure**: Concrete implementations (IO, LLM, Windowing*).
4.  **Presentation (UI/World3D)**:
    -   `IWorldView`: Interface for visualizing the world.
    -   `VulkanEngine`: Concrete implementation of `IWorldView` for Mode A.
    -   `HeadlessView`: Stub/Logging implementation for Mode B.

## 4. Constraint Checklist

- [ ] `src/core` compilation must never require `vulkan.h`.
- [ ] `src/application` code must interact with `IWorldView`, never `Engine` directly.
- [ ] `SisterSTRATA_cli` must link and run without `libvulkan.so` present.
