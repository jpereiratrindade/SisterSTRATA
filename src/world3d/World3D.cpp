#include "world3d/include/world3d/World3D.hpp"
#include "world3d/Engine.hpp"
#include "core/domain/soils/SoilSystem.hpp"
#include <memory>
#include <iostream>

namespace World3D {

// The single global instance of the Engine
static std::unique_ptr<Engine> g_Engine;

void init(SDL_Window* window) {
    try {
        g_Engine = std::make_unique<Engine>();
        g_Engine->init(window);
        
        // Load demo data by default for now
        // g_Engine->uploadDemoData();
        
    } catch (const std::exception& e) {
        std::cerr << "[World3D] Failed to initialize: " << e.what() << std::endl;
        throw;
    }
}

void shutdown() {
    g_Engine->shutdown();
    g_Engine.reset();
}

void clear() {
    if (g_Engine) g_Engine->clear();
}

void loadDemoCloud() {
    if (g_Engine) {
        g_Engine->uploadDemoDataAsync();
    }
}

void loadFile(const std::string& path) {
    if (g_Engine) g_Engine->loadFile(path);
}

void loadPointCloud(const std::vector<Core::ValueObjects::Vector3>& points,
                    const std::vector<glm::vec3>& colors,
                    const std::string& label) {
    if (g_Engine) g_Engine->loadPointCloud(points, colors, label);
}

bool saveFile(const std::string& path) {
    if (g_Engine) return g_Engine->saveFile(path);
    return false;
}

std::string getCurrentFilePath() {
    if (g_Engine) return g_Engine->getCurrentFilePath();
    return "";
}

vk::Instance getInstance() { return g_Engine ? g_Engine->getContext()->getInstance() : nullptr; }
vk::PhysicalDevice getPhysicalDevice() { return g_Engine ? g_Engine->getContext()->getPhysicalDevice() : nullptr; }
vk::Device getDevice() { return g_Engine ? g_Engine->getContext()->getDevice() : nullptr; }
vk::Queue getGraphicsQueue() { return g_Engine ? g_Engine->getContext()->getGraphicsQueue() : nullptr; }
uint32_t getGraphicsQueueFamilyIndex() { return g_Engine ? g_Engine->getContext()->getGraphicsQueueFamilyIndex() : 0; }

vk::RenderPass getRenderPass() { return g_Engine ? g_Engine->getRenderer().getRenderPass() : nullptr; }
vk::DescriptorPool getDescriptorPool() { return g_Engine ? g_Engine->getDescriptorPool() : nullptr; }
uint32_t getMinImageCount() { return 2; }
uint32_t getImageCount() { return g_Engine ? g_Engine->getRenderer().getSwapchain().getImageCount() : 0; }
// Header said getImageCount. 
// VulkanRenderer has getSwapchain().getImageCount().
// Let's check VulkanRenderer.hpp accessors.
// It has getSwapchain().
// So: g_Engine->getRenderer().getSwapchain().getImageCount()

vk::CommandBuffer getCurrentCommandBuffer() {
    return g_Engine ? g_Engine->getRenderer().getCommandBuffers()[g_Engine->getRenderer().getCurrentFrameIndex()] : nullptr;
}


void beginFrame() { 
    if (g_Engine) {
        g_Engine->update(1.0f / 60.0f); 
    }
}

void processEvent(const SDL_Event& event) {
    if (g_Engine) {
        g_Engine->processEvent(event);
    }
}

void endFrame(std::function<void(vk::CommandBuffer)> overlayCallback) { 
    if (g_Engine) {
        g_Engine->render(overlayCallback);
    }
}

// Re-implementing wrappers to match previous behavior if possible, 
// OR simpler: The external app likely calls World3D::beginFrame() ... World3D::endFrame().
// But existing Engine::render() does both.
// Let's look at how World3D.hpp is used.
// Since I don't see main.cpp, I will assume a standard loop.
// I will temporarily make `beginFrame` call `Engine::update` and `endFrame` call `Engine::render`.

// Lighting Control
glm::vec3 getLightDirection() { return g_Engine ? g_Engine->getLightDirection() : glm::vec3(0.0f); }
void setLightDirection(float x, float y, float z) { if (g_Engine) g_Engine->setLightDirection(x, y, z); }
glm::vec3 getLightColor() { return g_Engine ? g_Engine->getLightColor() : glm::vec3(1.0f); }
void setLightColor(float r, float g, float b) { if (g_Engine) g_Engine->setLightColor(r, g, b); }
float getAmbientStrength() { return g_Engine ? g_Engine->getAmbientStrength() : 0.0f; }
void setAmbientStrength(float strength) { if (g_Engine) g_Engine->setAmbientStrength(strength); }
void setPointSize(float size) { if (g_Engine) g_Engine->setPointSize(size); }
bool applyPointCloudColorMode(int mode, const glm::vec3& color) { return g_Engine ? g_Engine->applyPointCloudColorMode(mode, color) : false; }
bool requestScreenshot(const std::string& path) { return g_Engine ? g_Engine->requestScreenshot(path) : false; }

void applySlopeAnalysis() { if (g_Engine) g_Engine->applySlopeVisualization(); }

void applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter) { 
    if (g_Engine) g_Engine->applySoilSimulation(params, visualizationLevel, filter); 
}

void applyClassificationVisualization(const std::vector<int>& semanticMap) {
    if (g_Engine) g_Engine->applyClassificationVisualization(semanticMap);
}

void applyVegetationVisualization(const Core::Domain::Vegetation::VegetationOriginal& hypothesis, const std::vector<bool>& mask, bool accumulative) {
    if (g_Engine) g_Engine->applyVegetationVisualization(hypothesis, mask, accumulative);
}

void resetVisualization() {
    if (g_Engine) g_Engine->resetVisualization();
}

DrainageStats applyDrainageSimulation() {
    if (!g_Engine) return {};
    auto eStats = g_Engine->applyDrainageSimulation();
    World3D::DrainageStats stats;
    stats.maxAccumulation = eStats.maxAccumulation;
    stats.meanAccumulation = eStats.meanAccumulation;
    stats.riverCells = eStats.riverCells;
    stats.message = eStats.message;
    return stats;
}

bool setDrainageVisualization(bool showDrainage, bool showWatersheds, bool showBasinOutlines, float intensity) {
    if (!g_Engine) return false;
    return g_Engine->setDrainageVisualization(showDrainage, showWatersheds, showBasinOutlines, intensity);
}

HydrologyStats getHydrologyStats(float streamThreshold) {
    if (!g_Engine) return {};
    return g_Engine->getHydrologyStats(streamThreshold);
}

std::pair<bool, std::string> generateHydrologyReport(const std::string& path, float streamThreshold) {
    if (!g_Engine) return {false, "Engine not initialized"};
    return g_Engine->generateHydrologyReport(path, streamThreshold);
}

std::pair<bool, std::string> exportBasinBoundariesCsv(const std::string& path) {
    if (!g_Engine) return {false, "Engine not initialized"};
    return g_Engine->exportBasinBoundariesCsv(path);
}

SlopeStats getSlopeAnalysisStats() {
    if (g_Engine) {
        auto s = g_Engine->getSlopeAnalysisStats();
        return {s.countFlat, s.countGentle, s.countModerate, s.countSteep, s.total};
    }
    return {};
}

bool saveReport(const std::string& path) {
    if (g_Engine) return g_Engine->saveSlopeStats(path);
    return false;
}

bool generateTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad) {
    if (g_Engine) return g_Engine->generateSampleTerrain(filename, width, height, spacing, type, autoLoad);
    return false;
}

bool isTerrainGenerating() {
    if (g_Engine) return g_Engine->isTerrainGenerating();
    return false;
}

float getGenerationProgress() {
    if (g_Engine) return g_Engine->getGenerationProgress();
    return 0.0f;
}

std::string getGenerationMessage() {
    if (g_Engine) return g_Engine->getGenerationMessage();
    return "";
}
void setVSync(bool enabled) {
    if (g_Engine) g_Engine->setVSync(enabled);
}

bool getVSync() {
    if (g_Engine) return g_Engine->getVSync();
    return true;
}

void setTargetFPS(int fps) {
    if (g_Engine) g_Engine->setTargetFPS(fps);
}

int getTargetFPS() {
    if (g_Engine) return g_Engine->getTargetFPS();
    return 0;
}

int getPickIndex(float mouseX, float mouseY, int screenWidth, int screenHeight) {
    if (g_Engine) return g_Engine->getPickIndex(mouseX, mouseY, screenWidth, screenHeight);
    return -1;
}

void highlightPatch(const std::vector<uint32_t>& labels, int patchId) {
    if (g_Engine) g_Engine->highlightPatch(labels, patchId);
}

void setCameraSpeed(float speed) {
    if (g_Engine) g_Engine->setCameraSpeed(speed);
}


const std::vector<World3D::Rendering::Vertex>& getVertices() {
    static const std::vector<World3D::Rendering::Vertex> empty;
    return g_Engine ? g_Engine->getActiveVertices() : empty;
}

const std::vector<Core::Domain::Soils::SiBCSClassification>& getSoilClasses() {
    return Core::Domain::Soils::SoilSystem::getLastClassMap();
}

const Core::Domain::Hydro::HydroGrid& getHydroGrid() {
    // Return a static empty grid if engine not ready, or the engine's grid
    static Core::Domain::Hydro::HydroGrid empty;
    return g_Engine ? g_Engine->getHydroGrid() : empty;
}

} // namespace World3D
