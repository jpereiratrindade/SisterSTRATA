#pragma once
#include <vector>
#include <string>
#include "core/value_objects/TerrainVertex.hpp"
#include "core/domain/spatial_pattern/PatchAnalysis.hpp" // For GridData struct
#include "core/domain/soils/SiBCS.hpp"

namespace Core::Domain::SpatialPattern {

class SoilRasterizer {
public:
    /**
     * @brief Rasterizes a set of vertices into a 2D grid based on world coordinates.
     * 
     * @param vertices The input vertices containing position and color/soil info.
     *                 NOTE: Currently we assume soil class is encoded in vertex color or we need to separate params?
     *                 Ideally, we should pass the Soil Classification map directly if available, 
     *                 but SoilSystem stores it alongside vertices. 
     *                 If we only have colored vertices, we might need to map color back to class 
     *                 or accept a parallel vector of classifications.
     *                 For now, let's assume we can map the vertex storage or pass the classes explicitly.
     * 
     * @param classes Parallel vector of soil classifications for each vertex. 
     *                If empty, we might try to infer from color (risky).
     *                Better: We pass the vertices and the classification list that matches them.
     * 
     * @param cellSize The size of each grid cell in world units.
     * @return GridData The resulting grid compatible with PatchAnalysis.
     */
    static GridData Rasterize(const std::vector<Core::ValueObjects::TerrainVertex>& vertices, 
                              const std::vector<Core::Domain::Soils::SiBCSClassification>& classes,
                              double cellSize);

    /**
     * @brief Helper to generate the Legend CSV for the rasterized grid.
     */
    static void SaveLegendCsv(const std::string& path, const std::vector<Core::Domain::Soils::SiBCSClassification>& uniqueClasses);
};

} // namespace Core::Domain::SpatialPattern
