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
#include <glm/glm.hpp> // Added this include as it's implied by the instruction snippet but not in original

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
    
    // Callbacks for UI reporting
    std::function<void(const std::string&)> onStatusMessage;

    // Lighting Control
    void setLightDirection(float x, float y, float z);
    void setLightColor(float r, float g, float b);
    void setAmbientStrength(float strength);
    
    const glm::vec3& getLightDirection() const { return lightDir_; }
    const glm::vec3& getLightColor() const { return lightColor_; }
    float getAmbientStrength() const { return ambientStrength_; }

    // Analysis
    void applySlopeVisualization();

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
    bool generateSampleTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad = true);
    bool isTerrainGenerating() const { return isGenerating_; }

    // Analysis
    bool saveSlopeStats(const std::string& filepath);

    // Settings
    void setCameraSpeed(float speed);

private:
    void notifyStatus(const std::string& msg);
    void uploadReferenceGrid();

    SlopeStats lastStats_;
    std::atomic<bool> isGenerating_{false};

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
    std::string currentFilePath_; // New

    bool framebufferResized_ = false;

    // Lighting Data
    glm::vec3 lightDir_ = glm::vec3(0.5f, 1.0f, 2.0f);
    glm::vec3 lightColor_ = glm::vec3(1.0f);
    float ambientStrength_ = 0.3f;
};


} // namespace World3D
