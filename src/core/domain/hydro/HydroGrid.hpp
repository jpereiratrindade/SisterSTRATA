#pragma once

#include <vector>
#include <cstdint>
#include "core/domain/hydro/HydroTypes.hpp"

namespace Core::Domain::Hydro {

/**
 * @brief Represents a hydrological analysis grid.
 */
struct HydroGrid {
    int width = 0;
    int height = 0;
    
    // D8 Flow Direction
    std::vector<FlowDir> flowDirection; 

    // Receiver index per cell (-1 = sink)
    std::vector<int> receiverIndex;
    
    // Accumulated Flow (count of upstream cells, including self or not depending on semantic)
    // Currently: Includes self (1 unit per cell)
    std::vector<int32_t> flowAccumulationCells;
    
    // Future metrics (placeholders)
    std::vector<float> slope; 
    std::vector<float> specificCatchmentArea; 

    // Visualization
    std::vector<float> waterDepth; 

    // Watershed segmentation (basin IDs, 0 = unassigned)
    std::vector<int> watershedMap;

    bool isValid() const { return width > 0 && height > 0 && !flowDirection.empty(); }
};

} // namespace Core::Domain::Hydro
