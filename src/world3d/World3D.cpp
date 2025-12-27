#include "world3d/include/world3d/World3D.hpp"
#include "world3d/Engine.hpp"
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

void applySlopeAnalysis() { if (g_Engine) g_Engine->applySlopeVisualization(); }

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

void setCameraSpeed(float speed) {
    if (g_Engine) g_Engine->setCameraSpeed(speed);
}

} // namespace World3D
