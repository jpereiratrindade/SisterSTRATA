#pragma once

#include <vector>
#include <cstdint>
#include "core/domain/hydro/HydroGrid.hpp"

namespace Core::Domain::Hydro {

class Watershed {
public:
    // Delineate upstream basin from a seed and optionally tag basin ID.
    static std::vector<uint8_t> delineate(HydroGrid& grid, int startX, int startY, int basinID = 0);

    // Segment all basins using sinks as outlets. Returns basin count.
    static int segmentGlobal(HydroGrid& grid);
};

} // namespace Core::Domain::Hydro
