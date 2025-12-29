#pragma once

#include <functional> // Added
#include <vector>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>
#include "core/domain/soils/Scorpan.hpp" // New
#include "core/domain/soils/SiBCS.hpp"


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

// Graphics Settings
void setVSync(bool enabled);
bool getVSync();
void setTargetFPS(int fps);
int getTargetFPS();

// Lighting Control
/**
 * @brief Statistics report for terrain slope classification.
 */
struct SlopeStats {
    int countFlat;     ///< Vertices with slope < 5 degrees
    int countGentle;   ///< Vertices with slope 5-20 degrees
    int countModerate; ///< Vertices with slope 20-45 degrees
    int countSteep;    ///< Vertices with slope > 45 degrees
    int total;         ///< Total analyzed vertices
};
void applySlopeAnalysis();


/**
 * @brief Runs the Soil Prediction algorithm on the current active terrain.
 * 
 * @param params Global SCORPAN parameters.
 * @param visualizationLevel Level of detail for coloring (1=Order, ..., 6=Series).
 * @param filter Active filter to show/hide specific classes.
 */
void applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter);

SlopeStats getSlopeAnalysisStats();
bool saveReport(const std::string& path);

// Tools
/**
 * @brief Generates a new procedural terrain.
 * 
 * @param filename File path to save OBJ.
 * @param width Grid width.
 * @param height Grid height.
 * @param spacing Grid spacing.
 * @param type Terrain Pattern (0=Flat, 1=Hills, 2=Mountains, 3=Canyon, 4=Showcase).
 * @param autoLoad Whether to load the generated file immediately.
 * @return true if successful.
 */
bool generateTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad);

bool isTerrainGenerating();
float getGenerationProgress();
std::string getGenerationMessage();
void setCameraSpeed(float speed);

} // namespace World3D
