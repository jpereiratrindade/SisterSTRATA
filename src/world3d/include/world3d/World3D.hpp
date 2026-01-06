#pragma once

#include <functional> // Added
#include <memory>
#include <vector>
#include <string>
#include "world3d/rendering/Vertex.hpp"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>
#include "core/value_objects/Vector3.hpp"
#include "core/domain/soils/Scorpan.hpp" // New
#include "core/domain/soils/SiBCS.hpp"
#include "core/domain/hydro/HydrologyReport.hpp"
#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/vegetation/VegetationOriginal.hpp"


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
void loadPointCloud(const std::vector<Core::ValueObjects::Vector3>& points,
                    const std::vector<glm::vec3>& colors,
                    const std::string& label);
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
void setPointSize(float size);
/**
 * @brief Apply a color mode to the active point/line object.
 * @param mode 0 = use original source colors, 1 = override with a single color.
 * @param color Override color when mode = 1.
 */
bool applyPointCloudColorMode(int mode, const glm::vec3& color);
bool requestScreenshot(const std::string& path);

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
struct DrainageStats {
    int maxAccumulation = 0;
    float meanAccumulation = 0.0f;
    int riverCells = 0;
    std::string message;
};

using HydrologyStats = ::Core::Domain::Hydro::HydrologyStats;

/**
 * @brief Runs D8 drainage analysis.
 * @return Statistics of the run.
 */
DrainageStats applyDrainageSimulation();
/**
 * @brief Toggle drainage and watershed visualization overlays.
 */
bool setDrainageVisualization(bool showDrainage, bool showWatersheds, bool showBasinOutlines, float intensity);
/**
 * @brief Compute hydrology statistics for the current grid.
 */
HydrologyStats getHydrologyStats(float streamThreshold);
/**
 * @brief Generate a hydrology report to disk.
 */
std::pair<bool, std::string> generateHydrologyReport(const std::string& path, float streamThreshold);
/**
 * @brief Export basin boundary polylines to CSV.
 * Format: line_id, seq, x, y, z, r, g, b, basin_id.
 */
std::pair<bool, std::string> exportBasinBoundariesCsv(const std::string& path);


/**
 * @brief Runs the Soil Prediction algorithm on the current active terrain.
 * 
 * @param params Global SCORPAN parameters.
 * @param visualizationLevel Level of detail for coloring (1=Order, ..., 6=Series).
 * @param filter Active filter to show/hide specific classes.
 */
void applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter);
void applyClassificationVisualization(const std::vector<int>& semanticMap);
/**
 * @brief Applies vegetation color visualization to the current mesh.
 * @param hypothesis The hypothesis configuration (Vegetation Type, Slope Limits, etc.).
 * @param mask Pre-calculated coverage mask (true = covered).
 * @param accumulative If true, does not reset non-matching pixels to base color.
 */
void applyVegetationVisualization(const ::Core::Domain::Vegetation::VegetationOriginal& hypothesis, const std::vector<bool>& mask, bool accumulative = false);
void resetVisualization();

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

// Analysis Accessors
const std::vector<World3D::Rendering::Vertex>& getVertices(); // New
const std::vector<Core::Domain::Soils::SiBCSClassification>& getSoilClasses(); // New
const Core::Domain::Hydro::HydroGrid& getHydroGrid(); // New

int getPickIndex(float mouseX, float mouseY, int screenWidth, int screenHeight);

void highlightPatch(const std::vector<uint32_t>& labels, int patchId);

void setCameraSpeed(float speed);

} // namespace World3D
