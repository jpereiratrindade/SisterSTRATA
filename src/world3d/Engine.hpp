#pragma once

#include "world3d/rendering/VulkanContext.hpp"
#include "world3d/rendering/VulkanRenderer.hpp"
#include "world3d/camera/Camera.hpp"
#include "world3d/camera/CameraInputController.hpp"
#include "world3d/scene/Scene.hpp"
#include "infrastructure/threading/ThreadPool.hpp"
#include "infrastructure/threading/CommandQueue.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <functional>
#include <string> // Added this include as it's implied by the instruction snippet but not in original
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <glm/glm.hpp> 
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"
#include "core/domain/hydro/HydroGrid.hpp" // Added
#include "core/domain/hydro/HydrologyReport.hpp"
#include "core/value_objects/Vector3.hpp"

namespace World3D {

class Engine {
public:
    Engine();
    ~Engine();

    void init(SDL_Window* window);
    void shutdown();
    void clear(); // Clear scene
    
    void processEvent(const SDL_Event& event);
    void update(float deltaTime);
    void render(std::function<void(vk::CommandBuffer)> overlayRender = nullptr);


    // Accessors for Legacy Facade
    std::shared_ptr<Rendering::VulkanContext> getContext() { return context_; }
    Rendering::VulkanRenderer& getRenderer() { return *renderer_; }
    vk::DescriptorPool getDescriptorPool() { return descriptorPool_; }

    void uploadDemoData(); 
    void uploadDemoDataAsync(); 

    void loadFile(const std::string& path); // Generic loader
    void loadPointCloud(const std::vector<Core::ValueObjects::Vector3>& points,
                        const std::vector<glm::vec3>& colors,
                        const std::string& label);
    
    // Callbacks for UI reporting
    std::function<void(const std::string&)> onStatusMessage;

    // Lighting Control
    void setLightDirection(float x, float y, float z);
    void setLightColor(float r, float g, float b);
    void setAmbientStrength(float strength);
    void setPointSize(float size);
    /**
     * @brief Apply a color mode to the active point or line object.
     * @param mode 0 = use original source colors, 1 = override with a single color.
     * @param color Override color when mode = 1.
     * @return true if a compatible active object was updated.
     */
    bool applyPointCloudColorMode(int mode, const glm::vec3& color);
    bool requestScreenshot(const std::string& path);
    
    const glm::vec3& getLightDirection() const { return lightDir_; }
    const glm::vec3& getLightColor() const { return lightColor_; }
    float getAmbientStrength() const { return ambientStrength_; }





// ...

    // Analysis
    void applySlopeVisualization();
    void applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter);
    struct DrainageStats {
        int maxAccumulation = 0;
        float meanAccumulation = 0.0f;
        int riverCells = 0;
        std::string message;
    };

    /**
     * @brief Applies drainage simulation on the active terrain.
     * @return Statistics of the simulation.
     */
    DrainageStats applyDrainageSimulation();
    /**
     * @brief Toggle drainage and watershed visualization overlays.
     */
    bool setDrainageVisualization(bool showDrainage, bool showWatersheds, bool showBasinOutlines, float intensity);
    /**
     * @brief Compute hydrology statistics using current grid state.
     */
    ::Core::Domain::Hydro::HydrologyStats getHydrologyStats(float streamThreshold);
    /**
     * @brief Generate a hydrology report to disk.
     */
    bool generateHydrologyReport(const std::string& filepath, float streamThreshold);
    /**
     * @brief Export basin boundary polylines to CSV.
     * Format: line_id, seq, x, y, z, r, g, b, basin_id.
     */
    bool exportBasinBoundariesCsv(const std::string& filepath);

    const ::Core::Domain::Hydro::HydroGrid& getHydroGrid() const { return lastHydroGrid_; }

    struct SlopeStats {
        int countFlat = 0;
        int countGentle = 0;
        int countModerate = 0;
        int countSteep = 0;
        int total = 0;
    };
    
    SlopeStats getSlopeAnalysisStats() const { return lastStats_; }
    // IO
    bool saveFile(const std::string& path); 
    std::string getCurrentFilePath() const { return currentFilePath_; }

    // Terrain
    /**
     * @brief Generates a sample terrain asynchronously.
     * @param filename Output path (OBJ).
     * @param width Width in vertices.
     * @param height Height in vertices.
     * @param spacing Vertex spacing.
     * @param type Terrain Type (Flat, Hills, etc.).
     * @param autoLoad If true, automatically queues a loadFile request upon completion.
     * @return true if parameters are valid and system is ready.
     */
    bool generateSampleTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad = true);
    
    /**
     * @brief Checks if the system is currently busy generating or loading terrain.
     * @return true if busy.
     */
    bool isTerrainGenerating() const { return isGenerating_ || isLoading_; }
    
    /**
     * @brief Gets the current generation progress (0.0 to 1.0).
     */
    float getGenerationProgress() const { return generationProgress_; }

    /**
     * @brief Gets the current status message (e.g., "Generating Vertices", "Loading...").
     */
    std::string getGenerationMessage() const;

    // Analysis
    bool saveSlopeStats(const std::string& filepath);

    // Settings
    void setCameraSpeed(float speed);
    
    void setVSync(bool enabled);
    bool getVSync() const;

    void setTargetFPS(int fps);
    int getTargetFPS() const { return targetFps_; }

    // Analysis Access
    const std::vector<Rendering::Vertex>& getActiveVertices() const { 
        static const std::vector<Rendering::Vertex> empty;
        return activeVertices_ ? *activeVertices_ : empty; 
    }

private:
    void notifyStatus(const std::string& msg);
    void uploadReferenceGrid();
    void limitFrameRate(); // New helper

    int targetFps_ = 0; // 0 = unlimited
    std::chrono::steady_clock::time_point lastFrameTime_;

    SlopeStats lastStats_;
    DrainageStats lastDrainageStats_;
    ::Core::Domain::Hydro::HydrologyStats lastHydrologyStats_;
    ::Core::Domain::Hydro::HydroGrid lastHydroGrid_; // Added
    std::atomic<bool> isGenerating_{false};
    std::atomic<bool> isLoading_{false};
    std::atomic<float> generationProgress_{0.0f};
    std::string generationMessage_;
    mutable std::mutex generationMutex_;

    // Systems
    std::shared_ptr<Rendering::VulkanContext> context_;
    std::unique_ptr<Rendering::VulkanRenderer> renderer_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<CameraInputController> inputController_;
    std::unique_ptr<Infrastructure::Threading::ThreadPool> threadPool_;
    Infrastructure::Threading::CommandQueue commandQueue_;
    
    Scene scene_;
    vk::DescriptorPool descriptorPool_; // Managed by Engine for strict RAII
    
    // Active Data for Analysis/Export
    std::shared_ptr<std::vector<Rendering::Vertex>> activeVertices_;
    std::shared_ptr<Rendering::Buffer> activeVertexBuffer_;
    vk::PrimitiveTopology activeTopology_ = vk::PrimitiveTopology::eTriangleList; // New
    std::vector<glm::vec3> activeOriginalColors_;
    std::vector<glm::vec3> activeOriginalColors_;
    std::string currentFilePath_; // New

    bool framebufferResized_ = false;

    // Lighting Data
    glm::vec3 lightDir_ = glm::vec3(0.5f, 1.0f, 2.0f);
    glm::vec3 lightColor_ = glm::vec3(1.0f);
    float ambientStrength_ = 0.3f;

    enum class HydroVisMode { None, Drainage, Watershed };
    HydroVisMode hydroVisMode_ = HydroVisMode::None;
    std::vector<glm::vec3> baseColors_;
    bool baseColorsValid_ = false;
};


} // namespace World3D
