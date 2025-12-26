#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_events.h>

struct SDL_Window;

namespace World3D {

void init(SDL_Window* window);
void shutdown();

// Accessors for Main Loop Integration
vk::Instance getInstance();
vk::PhysicalDevice getPhysicalDevice();
vk::Device getDevice();
vk::Queue getGraphicsQueue();
uint32_t getGraphicsQueueFamilyIndex();

// Renderer Access
vk::RenderPass getRenderPass();
vk::DescriptorPool getDescriptorPool();
uint32_t getMinImageCount();
uint32_t getImageCount();

void loadDemoCloud(); // New test function

void processEvent(const SDL_Event& event); // New input handler

void beginFrame();
void endFrame();
vk::CommandBuffer getCurrentCommandBuffer();

} // namespace World3D
