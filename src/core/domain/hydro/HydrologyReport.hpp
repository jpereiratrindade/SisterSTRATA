#pragma once

#include <string>
#include <vector>
#include "core/domain/hydro/HydroTypes.hpp"
#include "core/domain/hydro/HydroGrid.hpp"

namespace Core::Domain::Hydro {

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
    int basinCount = 0;
    int largestBasinArea = 0;
    float largestBasinPct = 0.0f;

    // Per-basin detailed data
    int id = 0;
    int areaCells = 0;
    std::vector<HydrologyStats> topBasins;

    void initRanges();
};

class HydrologyReport {
public:
    static HydrologyStats analyze(const ElevationGrid& terrain,
                                  const HydroGrid& grid,
                                  float resolution,
                                  float streamThreshold = 100.0f);

    static bool generateToFile(const ElevationGrid& terrain,
                               const HydroGrid& grid,
                               float resolution,
                               const std::string& filepath,
                               float streamThreshold = 100.0f);
};

} // namespace Core::Domain::Hydro
