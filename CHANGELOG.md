# Changelog

All notable changes to the SisterPEC project will be documented in this file.

## [1.0.0] - 2025-12-26

### Added
- **Vulkan Rendering Engine**
    - High-performance forward renderer.
    - Support for Points, Lines, and Triangles.
    - Dynamic Lighting (Directional + Ambient).
    - Camera system with orbiting and free-look controls.
- **Scientific Data Support**
    - `ScientificAdapter` for handling large coordinate systems (Geological coordinates to GPU space).
    - Support for loading Point Clouds (CSV, XYZ, TXT).
    - Support for loading meshes (OBJ).
- **Asynchronous Architecture**
    - ThreadPool integration for non-blocking data loading.
    - `CommandQueue` for thread-safe cross-thread communication.
- **User Interface (UI)**
    - Implemented using ImGui with a "Professional Dark" theme.
    - **Tools > Settings** dialog for runtime configuration.
    - Lighting controls (Direction, RGB Color, Intensity).
    - Welcome panel with FPS visualization.
- **Project Structure**
    - Domain-Driven Design (DDD) inspired architecture.
    - Modular CMake build system (Core, Application, Infrastructure, UI, World3D).

### Fixed
- Resolved compilation issues in `VulkanRenderer`.
- Fixed runtime assertions in `ImGui` related to window management.

### Changed
- Migrated lighting controls from Welcome panel to dedicated Settings dialog.
