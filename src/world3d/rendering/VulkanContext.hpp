#pragma once

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <string>
#include <iostream>

namespace World3D::Rendering {

class VulkanContext {
public:
    VulkanContext(SDL_Window* window, const std::string& appName);
    ~VulkanContext();

    [[nodiscard]] vk::Device getDevice() const { return device_; }
    [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    [[nodiscard]] vk::Instance getInstance() const { return instance_; }
    [[nodiscard]] vk::SurfaceKHR getSurface() const { return surface_; }
    [[nodiscard]] vk::Queue getGraphicsQueue() const { return graphicsQueue_; }
    [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex_; }

private:
    void createInstance(SDL_Window* window, const std::string& appName);
    void setupDebugMessenger();
    void createSurface(SDL_Window* window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions(SDL_Window* window);

    vk::Instance instance_;
    vk::DebugUtilsMessengerEXT debugMessenger_;
    vk::SurfaceKHR surface_;
    
    vk::PhysicalDevice physicalDevice_;
    vk::Device device_;
    
    vk::Queue graphicsQueue_;
    uint32_t graphicsQueueFamilyIndex_ = -1;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

} // namespace World3D::Rendering
