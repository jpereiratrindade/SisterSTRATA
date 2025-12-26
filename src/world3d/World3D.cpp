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

void loadDemoCloud() {
    // Now handled in init or via Engine API
    if (g_Engine) {
        g_Engine->uploadDemoDataAsync();
    }
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

} // namespace World3D
