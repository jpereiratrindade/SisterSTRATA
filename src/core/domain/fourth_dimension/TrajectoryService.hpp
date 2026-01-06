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
     * @brief Captures the current state using direct semantic codes (Campestre, Forest, etc).
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
        
        TimeSlice slice(id, ordinal, semanticState, waterMask, metadata, ClassificationType::SemanticCode);
        trajectory.addTimeSlice(slice);
    }

    /**
     * @brief Captures the current state from scenario indices (Hypothesis_01, etc).
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
        
        int ordinal = trajectory.getNextOrdinal();
        int id = ordinal;

        std::vector<int> scenarioData = scenarioIndices; // Indices are stored directly
        TimeSlice slice(id, ordinal, scenarioData, waterMask, metadata, ClassificationType::ScenarioIndex);
        trajectory.addTimeSlice(slice);
    }
};

} // namespace Core::Domain::FourthDimension
