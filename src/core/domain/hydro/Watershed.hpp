#pragma once

#include <vector>
#include <cstdint>
#include "core/domain/hydro/HydroGrid.hpp"

namespace Core::Domain::Hydro {

class Watershed {
public:
    /**
     * @brief Delineate upstream basin from a seed and optionally tag basin ID.
     * @param grid Hydro grid with receiverIndex populated.
     * @param startX Seed x.
     * @param startY Seed y.
     * @param basinID Optional basin ID to assign (0 = no assignment).
     * @return Mask of basin cells (255 = inside).
     */
    static std::vector<uint8_t> delineate(HydroGrid& grid, int startX, int startY, int basinID = 0);

    /**
     * @brief Segment all basins using sinks as outlets.
     * @param grid Hydro grid with receiverIndex populated.
     * @return Number of basins detected.
     */
    static int segmentGlobal(HydroGrid& grid);

    /**
     * @brief Compute a mask of basin boundaries (1 = boundary, 0 = interior).
     */
    static std::vector<uint8_t> computeBoundaryMask(const HydroGrid& grid);
};

} // namespace Core::Domain::Hydro
