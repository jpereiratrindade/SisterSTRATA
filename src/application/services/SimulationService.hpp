#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/domain/fourth_dimension/TimeSlice.hpp"
#include <vector>
#include <string>

namespace Application::Services {

/**
 * @brief Service for running predefined simulation scenarios.
 *
 * Extracted from Session.hpp to separate simulation logic
 * from session management.
 */
class SimulationService {
public:
    enum class SimulationType {
        Stability,
        Fragmentation,
        Deforestation
    };

    /**
     * @brief Run a simulation scenario, populating the given trajectory.
     * @param type The simulation type to run.
     * @param workspace The workspace containing the world.
     * @param trajectory Output trajectory that will be populated.
     */
    static void simulateCondition(SimulationType type,
                                  Core::Domain::Workspace& workspace,
                                  Core::Domain::FourthDimension::Trajectory& trajectory);
};

} // namespace Application::Services
