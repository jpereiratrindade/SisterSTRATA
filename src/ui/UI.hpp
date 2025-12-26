#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

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
    
    void beginFrame();
    void render(vk::CommandBuffer cmd); // New render takes command buffer
    void endFrame(); 

    void processEvent(const SDL_Event* event);
};

} // namespace UI
