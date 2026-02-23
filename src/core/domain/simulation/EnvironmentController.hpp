#pragma once
#include <vector>
#include <string>
#include "../infrastructure/InfrastructureTypes.hpp"
#include "../infrastructure/InfrastructureOrchestrator.hpp"

namespace strata::domain::simulation {

class EnvironmentController {
public:
    EnvironmentController(int days_to_simulate);

    // Runs the simulation and logs results to CSV
    void run(infrastructure::InfrastructureOrchestrator& orchestrator, const std::string& csv_path);

private:
    int total_days_;

    // Deterministic generation of environment data based on day index
    infrastructure::EcologicalInput generateDeterministicEcoInput(int day);
};

} // namespace strata::domain::simulation
