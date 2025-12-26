# SisterPEC Architecture Documentation

## Overview
SisterPEC is a high-performance Scientific Data Platform built on C++20 and Vulkan. It simulates the structure of a professional engine, tailored for scientific visualization rather than general game development.

## Core Pillars
This engine is considered **Robust** and **Professional** for the following reasons:

### 1. Robustness (Stability & Correctness)
*   **RAII Resource Management**: All Vulkan resources (Buffers, Textures, Contexts) are wrapped in C++ classes (`VulkanRenderer`, `Swapchain`, `Buffer`). Destructors automatically clean up GPU memory, preventing leaks.
*   **Resize Handling**: The engine listens to OS events (`SDL_WINDOWEVENT`) and performs a full **Swapchain Recreation** on resize. This ensures the 3D viewport adapts without stretching (aspect ratio correction) and the UI remains crisp (pixel-perfect native rendering).
*   **Thread Safety**: Heavy operations (like Point Cloud generation) are offloaded to a **Thread Pool**, keeping the main UI thread responsive (60fps+) even while processing millions of data points.
*   **Input Handling**: A dedicated `InputController` separates raw SDL events from game logic, allowing for complex camera behaviors (Orbit/FreeFlight) that don't conflict with UI clicks.

### 2. Professional Architecture (Separation of Concerns)
*   **DDD Data Transfer (DTO Policy)**:
    *   The **UI Layer** (`src/ui`) is strictly decoupled from the **World Layer** (`src/world3d`).
    *   They communicate *only* via **Data Transfer Objects** (`Application::DTO::UIData`).
    *   *Benefit*: The UI doesn't know about Vulkan or 3D Math. It just displays data. You can rewrite the entire rendering engine without breaking the UI.
*   **Engine vs. Game Loop**:
    *   `World3D::Engine` encapsulates the complexity of the graphics API.
    *   `main.cpp` controls the high-level application lifecycle (Init -> Loop -> Shutdown), acting as the "Director".

### 3. UI Ux (User Experience)
*   **Floating Panel Layout**: Mirrors professional tools (like Blender/Maya/SisterAppPEC) by using persistent floating panels rather than complex docking systems that confuse users.
*   **Native Rendering**: Disabling OS bitmap scaling ensures text is readable on 4K monitors, relying on vector fonts (`DroidSans`/`Roboto`) for clean edges.

## Roadmap to "AAA" Engine Status
While robust, to reach a "General Purpose Commercial Engine" level, the following would be needed:
1.  **Material System**: Currently shaders are hardcoded in Pipelines. A data-driven Material system (loading `.mat` files) is needed.
2.  **Scene Graph**: A hierarchical Tree structure for objects (Parent -> Child transforms). Currently, we use a flat list in `Scene`.
3.  **Asset Manager**: A centralized cache for textures/models to avoid loading the same file twice.

## Conclusion
For a **Scientific Visualization Tool**, this architecture is **Excellent**. It prioritizes stability, data throughput, and UI responsiveness, avoiding the bloat of generic game engines.
