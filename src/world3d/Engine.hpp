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


#include <vector>

namespace World3D {

class Engine {
public:
    Engine();
    ~Engine();

    void init(SDL_Window* window);
    void shutdown();
    
    void processEvent(const SDL_Event& event);
    void update(float deltaTime);
    void render(std::function<void(vk::CommandBuffer)> overlayRender = nullptr);


    // Accessors for Legacy Facade
    std::shared_ptr<Rendering::VulkanContext> getContext() { return context_; }
    Rendering::VulkanRenderer& getRenderer() { return *renderer_; }
    vk::DescriptorPool getDescriptorPool() { return descriptorPool_; }

    void uploadDemoData(); 
    void uploadDemoDataAsync(); // New

private:
    std::shared_ptr<Rendering::VulkanContext> context_;
    std::unique_ptr<Rendering::VulkanRenderer> renderer_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<CameraInputController> inputController_;
    
    // Threading
    std::unique_ptr<Infrastructure::Threading::ThreadPool> threadPool_;
    Infrastructure::Threading::CommandQueue commandQueue_;

    
    Scene scene_; // New Scene member

    vk::DescriptorPool descriptorPool_;
    bool framebufferResized_ = false;
};


} // namespace World3D
