#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>
#include <functional>
#include "application/dtos/UIData.hpp"

namespace UI {

struct VulkanInitInfo {
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue queue;
    uint32_t queueFamily;
    vk::DescriptorPool descriptorPool;
    vk::RenderPass renderPass;
    uint32_t minImageCount;
    uint32_t imageCount;
};

class UserInterface {
public:
    void init(SDL_Window* window, const VulkanInitInfo& info);
    void shutdown();

    // Callbacks
    std::function<void()> onLoadDemo;
    std::function<void()> onExit;
    
    void beginFrame();
    void draw(const Application::DTO::UIData& data);
    void render(vk::CommandBuffer cmd); // Final backend flush
    void endFrame(); 

    void processEvent(const SDL_Event* event);

    bool wantsToCaptureMouse() const;
    bool wantsToCaptureKeyboard() const;

private:
    SDL_Window* window_ = nullptr;
    float dpiScale_ = 1.0f;
};

} // namespace UI
