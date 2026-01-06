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
     * @brief Captures the current state using direct semantic codes.
     * @param trajectory The trajectory aggregate to add the state to.
     * @param semanticState The semantic classification map (VegetationCode).
     * @param waterMask Mask indicating presence of water.
     * @param metadata Description for the time slice.
     */
    static void captureSemanticState(Trajectory& trajectory,
                                     const std::vector<int>& semanticState,
                                     const std::vector<bool>& waterMask,
                                     const std::string& metadata) {
        int ordinal = trajectory.getNextOrdinal();
        int id = ordinal; 
        
        TimeSlice slice(id, ordinal, semanticState, waterMask, metadata);
        trajectory.addTimeSlice(slice);
    }

    /**
     * @brief Captures the current state from scenario indices.
     * Legacy/Helper: Maps transient Scenario Indices to persistent Vegetation Codes.
     * @param trajectory The trajectory aggregate.
     * @param scenarioIndices Map of scenario indices (from global overlay).
     * @param system Reference to the vegetation system for mapping.
     * @param waterMask Mask indicating presence of water.
     * @param metadata Description for the time slice.
     */
    static void captureState(Trajectory& trajectory, 
                             const std::vector<int>& scenarioIndices,
                             const Core::Domain::Vegetation::VegetationSystemOriginal& system,
                             const std::vector<bool>& waterMask,
                             const std::string& metadata) {
        
        std::vector<int> semanticState;
        semanticState.reserve(scenarioIndices.size());
        
        for (int idx : scenarioIndices) {
            semanticState.push_back(idx); 
        }

        captureSemanticState(trajectory, semanticState, waterMask, metadata);
    }
};

} // namespace Core::Domain::FourthDimension
