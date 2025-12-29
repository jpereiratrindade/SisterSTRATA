#pragma once

#include <vector>
#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/hydro/HydroTypes.hpp" // For FlowDir, ElevationGrid

namespace Core::Domain::Hydro {

class DrainageSystem {
public:
    /**
     * @brief Processes the terrain to generate drainage data.
     * @param terrain Pure domain elevation model.
     * @param grid Output HydroGrid.
     * @param sinkMethod Strategy for handling local depressions.
     */
    static void process(const ElevationGrid& terrain, HydroGrid& grid, SinkHandling sinkMethod = SinkHandling::Ignore);

private:
    static void calculateFlowDirection(const ElevationGrid& terrain, HydroGrid& grid);
    static void calculateFlowAccumulation(HydroGrid& grid);
};

} // namespace Core::Domain::Hydro
