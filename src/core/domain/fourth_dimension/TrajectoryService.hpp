#pragma once

#include "Trajectory.hpp"
#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include <vector>
#include <string>

namespace Core::Domain::FourthDimension {

/**
 * @brief Domain Service responsible for capturing and validating states.
 * Acts as a Builder for TimeSlices within a Trajectory.
 */
class TrajectoryService {
public:
    /**
     * @brief Captures the current state and appends it to the trajectory.
     * Maps transient Hypothesis Indices to persistent Vegetation Codes.
     */
    static void captureState(Trajectory& trajectory, 
                             const std::vector<int>& scenarioIndices,
                             const Core::Domain::Vegetation::VegetationSystemOriginal& system,
                             const std::vector<bool>& waterMask,
                             const std::string& metadata) {
        
        const auto& hypotheses = system.getHypotheses();
        std::vector<int> semanticState;
        semanticState.reserve(scenarioIndices.size());
        
        // Map Index -> Code
        for (int idx : scenarioIndices) {
            if (idx >= 0 && idx < static_cast<int>(hypotheses.size())) {
                auto code = hypotheses[idx].getType().getCode();
                semanticState.push_back(static_cast<int>(code));
            } else {
                semanticState.push_back(-1); // None/Soil
            }
        }

        int ordinal = trajectory.getNextOrdinal();
        int id = ordinal; 
        
        TimeSlice slice(id, ordinal, semanticState, waterMask, metadata);
        trajectory.addTimeSlice(slice);
    }
};

} // namespace Core::Domain::FourthDimension
