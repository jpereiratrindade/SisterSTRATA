# Changelog

## [v1.9.3] - 2026-01-09
### Added
- **Trajectory Impact Profile**: Implemented `TrajectoryImpactAnalyzer` service to generate comprehensive impact reports based on simulation trajectories.
- **Simulation UX**: Added Stability, Fragmentation, and Deforestation simulation controls directly to the `TimelinePanel`.
- **Spatial Mapping**: Enhanced `Engine` to support correct visualization of simulation grids on high-resolution terrain meshes (handling vertex count mismatches).

### Fixed
- **Deforestation Visualization**: Resolved issue where deforestation appeared "All Green" by implementing spatial mapping and ensuring correct "Soil" color application.
- **Ghost Mode**: Fixed visualization persistence issues for simulated states.
- **Vulkan Stabilization (Patch 1)**: Implemented robust semaphore indexing (triple-buffering) and image-in-flight tracking to resolve validation errors on strict drivers (RTX 40xx / Wayland).
- **AI Connectivity**: Hardened Ollama connection logic to use `127.0.0.1` (IPv4) explicit timeouts, resolving connection failures on Fedora/Linux.
- **UI Consistency**: Restored integrated history lists in Discursive and Trajectory panels for better UX across development environments.
- **UI Layout**: Fixed positioning of "Propose System" and "Evaluate Coherence" buttons in Discursive Panel to prevent button clipping on smaller resolutions or high-DPI scaling.

### Documentation
- **User Manual**: Comprehensive guide (`docs/MANUAL_DO_USUARIO.md`) connected to scientific foundations.
- **Scientific Deep Dives**: Added/Linked docs for Hydrology, Vegetation, Fourth Dimension, and Impact Profile.
-   **Usage Examples**: Added `examples/data_samples/` with JSON/CSV datasets and a practical guide (`docs/EXEMPLOS_PRATICOS.md`).
-   **README**: Revamped with "Getting Started" and Feature Highlights.


### Added
- **Agent Guidance**: Added `AGENTS.md` with guardrails for automated assistants.
- **Scientific Docs**: New domain documents for Pedosfera, Hidrologia, Vegetação e Quarta Dimensão.
- **Doc Index**: Added `docs/README.md` for navigation.
- **Pedosfera Bibliografia**: Added `docs/SCIENTIFIC_PEDOSFERA_BIBLIO.md`.

### Changed
- **Documentation Links**: Converted references to clickable Markdown links.
- **Git Ignore**: Ignored local artifacts and refined exceptions for `assets/data`.

### Removed
- **Local Artifacts**: Removed large local datasets and generated outputs from version control.

## [v1.9.1] - 2026-01-06
### Added
- **Export Grid Recovery**: Smart CSV serialization that dedupes and sorts mesh vertices (OBJ) into valid soil grids for drainage usage.
- **Enhanced File Browser**: Added "Root" navigation shortcut (`/`) and editable path bar for full system access.

### Fixed
- **System Stability**: Resolved "Auto-Load" race condition in Terrain Simulator (preventing "System Busy" lockouts).
- **UX**: Fixed critical visualization bug where loading a project failed to initialize the 3D patch cache (resolved Proxy loading issue).

## [v1.9.0] - 2026-01-06
### Added
- **Full Project Persistence**: Save and load complete vegetation scenarios and trajectories using custom project names (e.g., `project_name.strata`).
- **3D Picking & Highlighting**: "Click-to-ID" functionality with robust ray-stepping for terrain relief and magenta visual feedback.
- **Detailed Landscape Statistics**: UI popup and LLM integration for Area (Mean/Min/Max/StdDev) and Shape Index distributions.
- **Trajectory Management**: Ability to remove individual time slices dynamically.

### Fixed
- **NaN Data Display**: Fixed issue where unloaded proxy slices caused NaN values in analysis reports.
- **Class 0 Detection**: Improved spatial engine to correctly identify patches of Class 0 (Campestre).
- **Metric Consistency**: Unified all AI-reported area metrics to Hectares (ha).

## [1.8.2] - 2026-01-05
### Added
- **Global Tactical Analysis**: Data-driven scene-wide interpretation of land-use trajectories across all captured states.
- **Hypothesis Semantic Grounding**: LLM prompts now resolve technical IDs to descriptive names and types (e.g., "FlorestalNatural").
- **Vector-Based Hypothesis Architecture**: Formalized the n-dimensional parameter space model in `docs/Arquitetura_Vetorial_Hipoteses.md`.
- **Patch Discovery UI**: Real-time display of available patch counts in the selected slice to guide ID selection.

### Fixed
- **UI Responsiveness**: Decoupled LLM request buffers for Hermeneutic (Pairwise) vs Trajectory (Global/Patch) to prevent UI deadlocks.
- **Robust Patch Tracking**: Support for "late-blooming" patches (seeding from any slice where the patch first appears).
- **Compilation**: Fixed `PatchState` aggregate initialization ambiguity in `TimelinePanel.cpp`.

## [1.8.1] - 2026-01-05
### Added
- **Multi-State Persistence**: Integrated `TrajectoryPersistenceService` with the standard `TimelinePanel` flow.
- **Dynamic Seeding**: Trajectory tracking now starts from the first detection slice instead of forcing T0.

## [1.8.0] - 2026-01-05
### Added
- **Material System**: Data-driven shaders and pipeline configuration via `Material` definitions.
- **LOD Temporal**: Memory optimization for trajectories using disk-backed persistence (`TrajectoryPersistenceService`).
- **Persistence SDK**: Binary serialization for `TimeSlice` states to scale historical analysis.
- **Patch Trajectory Analysis v1.1**: Full implementation of multi-state analysis aligned with DDD, including fractal dimension and structural stability.
- **Hermeneutic Context Enrichment**: LLM analysis now includes land-use distribution for both transition states, resolving interpretative ambiguity.
- **Patch Labeling**: Centroid calculation and visual ID labeling in the `Patch Analysis` preview for easy identification.

## [1.7.0] - 2026-01-05
### Added
- **Qwen LLM Integration**: Real-time hermeneutic analysis based on `Timeline` metrics.
- **Robust Model Discovery**: Automatically prioritizes the most powerful Qwen model available (e.g., 14b > 7b).
- **Resizable Insight Window**: Interactive UI for better reading of cognitive assistance reports.
- **Save Analysis Persistence**: Option to export AI-generated reports as timestamped `.txt` files.
- **Direct Ollama Connectivity**: Native C++ bridge using `cpp-httplib` and async worker threads.

## [1.6.0] - 2025-12-30

### Added
- **Fourth Dimension System (Resilience)**: New observational layer for temporal landscape analysis.
- **TimeSlice & Trajectory**: Implementation of immutable state snapshots and resilience trajectories.
- **Ghost Mode**: Non-destructive historic state visualization with stable semantic coloring.
- **Timeline UI**: Dedicated panel for state capture, trajectory navigation, and analysis.
- **Coherence Mapping**: Quantitative similarity analysis (Type, Structure, Edge) between landscape states.
- **Cognitive Assistance Context (CAC)**: Hybrid interpretative layer using Qwen (mock adapter) for hermeneutic analysis based on quantitative metrics.
- **Ports & Adapters Architecture**: Decoupled LLM integration with asynchronous infrastructure.

## [1.5.0] - 2025-12-28
### Added
- **Core Domain**: Initial structure with `Territory` aggregate root.
- **SiBCS Integration**: Brazilian Soil Classification System Level 1 and 2.
- **Hydrology**: Flow accumulation and drainage analysis.
- **Spatial Patterns**: `PatchAnalysis` for generic landscape metrics.
- **UI**: Modern Dark Theme with `ImGui` and HiDPI support.
- **Rendering**: 3D Point Cloud and Mesh visualization for terrain metadata.

## [1.4.0] - 2025-12-29
### Added
- **Enhanced Drainage Analysis**: New dialog in "Analyze Drainage" with flow statistics (Max/Mean Accumulation, River Cell count) and color ramp legends.
- **Improved 3D Rendering**: Enhanced point cloud visualization with depth-aware color ramps and adjustable point size for drainage patterns.
- **Native File Dialogs**: Integrated native-style file selection for Patch Analysis, improving usability and path safety.

### Changed
- **Namespace Refactor**: Migrated all core components from `farina::diversity` to `LandscapeEcologyTools` for better domain alignment.
- **Simplified Project Structure**: Relocated headers from `include/` to `src/`, streamlining the build system and developer experience.

## [1.3.0] - 2025-12-29
### Added
- **Unified File Browser**: Integrated the visual `FileSelector` component into the Main Menu for "Open File" and "Save As", replacing legacy text inputs.
- **Raster Grid Support**: "File > Open" now automatically detects and loads Soil Raster CSVs (`# Origin:` header), visualizing them as colored point clouds.

### Fixed
- **Stability**: Fixed a startup crash caused by static initialization order in the filesystem component.
- **UX**: Resolved issue where manual offsets were cancelled by the engine's auto-centering (Engine now uses absolute origin).

## [1.2.0] - 2025-12-29
### Added
- **Scientific Foundation**: Added `docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md` as the core "living document" for model governance.
- **Documentation**: Updated README to reflect the project's scientific nature.
- **Models**: Formalized SCORPAN (Soil), Conservation of Mass (Hydrology), and V-Cell (Vegetation) in the knowledge architecture.

## [1.1.0] - 2025-12-27

### Added
- **Application Class (`src/application`)**: New orchestration layer acting as the "Director", encapsulating the main loop and system lifecycle.
- **Security**: Added Path Traversal protection in `Engine::loadFile`.
- **Validation**: Added strict input validation for Terrain Generation.

### Fixed
- **Critical**: Fixed Race Condition in `ThreadPool` (lost wakeups).
- **Critical**: Fixed memory alignment for Shader Uniforms (`std140` compliance).
- **Stability**: Added exception safety to Engine callbacks.

### Changed
- **Refactoring**: Decoupled `main.cpp` from engine logic. Entry point is now minimal.
- **Code Quality**: Cleaned up `Buffer` class (removed unnecessary virtual destructor).

### Documentation
- Added comprehensive Code Review report (`docs/reviews/`).

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
