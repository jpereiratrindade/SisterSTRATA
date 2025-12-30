#pragma once

#include <string>
#include <vector>
#include "core/domain/hydro/HydroTypes.hpp"
#include "core/domain/hydro/HydroGrid.hpp"

namespace Core::Domain::Hydro {

/**
 * @brief Aggregated hydrology statistics for a grid or basin.
 */
struct HydrologyStats {
    // Structural
    float minElevation = 0.0f;
    float maxElevation = 0.0f;
    float avgElevation = 0.0f;

    float minSlope = 0.0f;
    float maxSlope = 0.0f;
    float avgSlope = 0.0f;

    // Functional
    float maxFlowAccumulation = 0.0f;
    float totalDischarge = 0.0f;
    float maxStreamPower = 0.0f;

    // Eco-hydrological
    float minTWI = 0.0f;
    float maxTWI = 0.0f;
    float avgTWI = 0.0f;
    float saturatedAreaPct = 0.0f;

    // Network
    float drainageDensity = 0.0f;
    int streamCount = 0;

    // Basins
    int basinCount = 0; ///< Number of basins detected.
    int largestBasinArea = 0; ///< Largest basin area in cells.
    float largestBasinPct = 0.0f; ///< Largest basin share (0-100%).

    // Per-basin detailed data
    int id = 0; ///< Basin identifier when stats are per-basin.
    int areaCells = 0; ///< Basin area in cells.
    std::vector<HydrologyStats> topBasins; ///< Largest basins by area.
    std::vector<HydrologyStats> allBasins; ///< All basin stats (per-basin).

    /**
     * @brief Initialize min/max ranges for aggregation.
     */
    void initRanges();
};

class HydrologyReport {
public:
    /**
     * @brief Analyze hydrologic metrics from elevation and flow data.
     * @param terrain Elevation grid.
     * @param grid Hydro grid with flow accumulation and basins.
     * @param resolution Cell spacing in world units.
     * @param streamThreshold Flow accumulation threshold (cells).
     * @return Aggregated hydrology statistics.
     */
    static HydrologyStats analyze(const ElevationGrid& terrain,
                                  const HydroGrid& grid,
                                  float resolution,
                                  float streamThreshold = 100.0f);

    /**
     * @brief Generate a formatted hydrology report and save to disk.
     * @param terrain Elevation grid.
     * @param grid Hydro grid with flow accumulation and basins.
     * @param resolution Cell spacing in world units.
     * @param filepath Output report path.
     * @param streamThreshold Flow accumulation threshold (cells).
     * @return true if report was saved successfully.
     */
    static bool generateToFile(const ElevationGrid& terrain,
                               const HydroGrid& grid,
                               float resolution,
                               const std::string& filepath,
                               float streamThreshold = 100.0f);
};

} // namespace Core::Domain::Hydro
