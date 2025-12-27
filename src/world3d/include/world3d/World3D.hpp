#pragma once

#include <functional> // Added
#include <vector>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>


struct SDL_Window;

namespace World3D {

void init(SDL_Window* window);
void shutdown();
void clear();

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
void loadFile(const std::string& path); // Generic loader
bool saveFile(const std::string& path); // Generic saver
std::string getCurrentFilePath(); // Get current active path

void processEvent(const SDL_Event& event); // New input handler

void beginFrame();
void endFrame(std::function<void(vk::CommandBuffer)> overlayCallback = nullptr);
vk::CommandBuffer getCurrentCommandBuffer();

// Lighting Control
glm::vec3 getLightDirection();
void setLightDirection(float x, float y, float z);
glm::vec3 getLightColor();
void setLightColor(float r, float g, float b);
float getAmbientStrength();
void setAmbientStrength(float strength);

// Analysis
struct SlopeStats {
    int countFlat;
    int countGentle;
    int countModerate;
    int countSteep;
    int total;
};
void applySlopeAnalysis();
SlopeStats getSlopeAnalysisStats();
bool saveReport(const std::string& path);
// Tools
bool generateTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad);
bool isTerrainGenerating();
void setCameraSpeed(float speed);

} // namespace World3D
