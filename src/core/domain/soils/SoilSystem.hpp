#pragma once
#include <vector>
#include "core/value_objects/TerrainVertex.hpp"
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"

namespace Core::Domain::Soils {

class SoilSystem {
public:
    /**
     * @brief Processes the terrain vertices and applies soil classification colors based on SCORPAN parameters.
     * 
     * @param vertices The vertex buffer to modify.
     * @param params The SCORPAN global parameters (Climate, Organisms, etc.).
     * @param visualizationLevel The SiBCS level to visualize (1=Order, 2=Suborder, 3=GreatGroup...)
     * @param filter An optional filter to isolate specific soil classes or levels.
     */
    static void process(std::vector<Core::ValueObjects::TerrainVertex>& vertices, const ScorpanParams& params, int visualizationLevel, const SiBCSFilter& filter);
    /**
     * @brief Retrieves the list of unique soil classes detected in the last simulation frame.
     * Used for populating the dynamic legend.
     */
    static const std::vector<SiBCSClassification>& getLastDetectedClasses();
    /**
     * @brief Retrieves the last per-vertex soil classifications.
     */
    static const std::vector<SiBCSClassification>& getLastClassMap();
    /**
     * @brief Clears any cached classes from the last simulation (legend reset).
     */
    static void clearLastDetectedClasses();

    
    /**
     * @brief Core prediction logic determining the soil class for a given point.
     * 
     * @param global Global SCORPAN parameters (Climate, Age, etc.).
     * @param slopeDeg Local slope in degrees.
     * @param elevation Absolute elevation.
     * @param relElevation Relative elevation (0..1) normalized to terrain min/max.
     * @return The calculated SiBCS classification.
     */
    static SiBCSClassification predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation);

private:
    static std::vector<SiBCSClassification> lastDetectedClasses_;
    static std::vector<SiBCSClassification> lastClassMap_;
};

} // namespace Core::Domain::Soils
