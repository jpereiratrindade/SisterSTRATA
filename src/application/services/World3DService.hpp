#pragma once

#include "world3d/World3D.hpp"
#include "world3d/adapters/TerrainVertexAdapter.hpp"
#include <vector>

namespace Application::Services {

class World3DService {
public:
    using SlopeStats = ::World3D::SlopeStats;
    using DrainageStats = ::World3D::DrainageStats;
    using HydrologyStats = ::World3D::HydrologyStats;

    static glm::vec3 getLightDirection() { return ::World3D::getLightDirection(); }
    static void setLightDirection(float x, float y, float z) { ::World3D::setLightDirection(x, y, z); }
    static glm::vec3 getLightColor() { return ::World3D::getLightColor(); }
    static void setLightColor(float r, float g, float b) { ::World3D::setLightColor(r, g, b); }
    static float getAmbientStrength() { return ::World3D::getAmbientStrength(); }
    static void setAmbientStrength(float strength) { ::World3D::setAmbientStrength(strength); }
    static void setPointSize(float size) { ::World3D::setPointSize(size); }
    static void setCameraSpeed(float speed) { ::World3D::setCameraSpeed(speed); }

    static bool applyPointCloudColorMode(int mode, const glm::vec3& color) {
        return ::World3D::applyPointCloudColorMode(mode, color);
    }

    static bool getVSync() { return ::World3D::getVSync(); }
    static void setVSync(bool enabled) { ::World3D::setVSync(enabled); }
    static int getTargetFPS() { return ::World3D::getTargetFPS(); }
    static void setTargetFPS(int fps) { ::World3D::setTargetFPS(fps); }

    static bool isTerrainGenerating() { return ::World3D::isTerrainGenerating(); }
    static float getGenerationProgress() { return ::World3D::getGenerationProgress(); }
    static std::string getGenerationMessage() { return ::World3D::getGenerationMessage(); }
    static bool generateTerrain(const std::string& filename, int width, int height, float spacing, int type, bool autoLoad) {
        return ::World3D::generateTerrain(filename, width, height, spacing, type, autoLoad);
    }

    static int getPickIndex(float mouseX, float mouseY, int screenWidth, int screenHeight) {
        return ::World3D::getPickIndex(mouseX, mouseY, screenWidth, screenHeight);
    }
    static void highlightPatch(const std::vector<uint32_t>& labels, int patchId) {
        ::World3D::highlightPatch(labels, patchId);
    }
    static void resetVisualization() { ::World3D::resetVisualization(); }

    static SlopeStats getSlopeAnalysisStats() { return ::World3D::getSlopeAnalysisStats(); }
    static void applySlopeAnalysis() { ::World3D::applySlopeAnalysis(); }
    static bool saveReport(const std::string& path) { return ::World3D::saveReport(path); }

    static void applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter) {
        ::World3D::applySoilSimulation(params, visualizationLevel, filter);
    }

    static void applyClassificationVisualization(const std::vector<int>& semanticMap) {
        ::World3D::applyClassificationVisualization(semanticMap);
    }

    static void applyVegetationVisualization(const ::Core::Domain::Vegetation::VegetationOriginal& hypothesis, const std::vector<bool>& mask, bool accumulative = false) {
        ::World3D::applyVegetationVisualization(hypothesis, mask, accumulative);
    }

    static DrainageStats applyDrainageSimulation() { return ::World3D::applyDrainageSimulation(); }
    static bool setDrainageVisualization(bool showDrainage, bool showWatersheds, bool showBasinOutlines, float intensity) {
        return ::World3D::setDrainageVisualization(showDrainage, showWatersheds, showBasinOutlines, intensity);
    }
    static HydrologyStats getHydrologyStats(float streamThreshold) { return ::World3D::getHydrologyStats(streamThreshold); }
    static std::pair<bool, std::string> generateHydrologyReport(const std::string& path, float streamThreshold) {
        return ::World3D::generateHydrologyReport(path, streamThreshold);
    }
    static std::pair<bool, std::string> exportBasinBoundariesCsv(const std::string& path) {
        return ::World3D::exportBasinBoundariesCsv(path);
    }

    static bool requestScreenshot(const std::string& path) { return ::World3D::requestScreenshot(path); }
    static void clear() { ::World3D::clear(); }

    static void loadPointCloud(const std::vector<Core::ValueObjects::Vector3>& points,
                               const std::vector<glm::vec3>& colors,
                               const std::string& label) {
        ::World3D::loadPointCloud(points, colors, label);
    }

    static std::string getCurrentFilePath() { return ::World3D::getCurrentFilePath(); }

    static const std::vector<World3D::Rendering::Vertex>& getVertices() { return ::World3D::getVertices(); }
    static std::vector<Core::ValueObjects::TerrainVertex> getTerrainVertices() {
        return ::World3D::Adapters::toTerrainVertices(::World3D::getVertices());
    }
    static const Core::Domain::Hydro::HydroGrid& getHydroGrid() { return ::World3D::getHydroGrid(); }
};

} // namespace Application::Services
