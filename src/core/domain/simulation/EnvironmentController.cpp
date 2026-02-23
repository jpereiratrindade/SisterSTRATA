#include "EnvironmentController.hpp"
#include <fstream>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace strata::domain::simulation {

EnvironmentController::EnvironmentController(
    int days_to_simulate,
    EnvironmentScenarioPreset scenario_preset)
    : total_days_(days_to_simulate)
    , scenario_preset_(scenario_preset)
{}

infrastructure::EcologicalInput EnvironmentController::generateDeterministicEcoInput(int day) {
    infrastructure::EcologicalInput eco;
    
    // Deterministic weather: sinusoidal solar generation over the year to simulate seasons
    // Let's say max summer is 5000 Wh, min winter is 1000 Wh
    double season_factor = (std::sin((day / 365.0) * 2.0 * M_PI) + 1.0) / 2.0; // [0, 1]
    eco.solar_wh = 1000.0 + (4000.0 * season_factor);

    // Add some deterministic "cloudy" days (e.g., every 7 days is a bad day)
    if (day % 7 == 0) {
        eco.solar_wh *= 0.3; // 70% drop
    }

    // Deterministic animal density: constant base with periodic spikes
    eco.animal_density = 50.0; // 50 animals base
    if (day % 30 == 0) {
        eco.animal_density = 150.0; // migration spike
    }

    // Deterministic soil moisture: lags behind rain, let's make it inverse to solar to create tension
    eco.soil_moisture = 1.0 - season_factor; // dryer in summer
    // Give it a boost on cloudy days
    if (day % 7 == 0) {
        eco.soil_moisture = std::min(1.0, eco.soil_moisture + 0.4);
    }

    // Scenario preset modifications (deterministic)
    if (scenario_preset_ == EnvironmentScenarioPreset::SevereDrought) {
        // Persistently low generation and drier conditions to force
        // energy stress and expose resilience behavior.
        eco.solar_wh *= 0.25;
        eco.soil_moisture = std::clamp(eco.soil_moisture - 0.35, 0.0, 1.0);

        // Additional stress every 15 days (dust/cloud bursts).
        if (day % 15 == 0) {
            eco.solar_wh *= 0.5;
        }
    }

    return eco;
}

void EnvironmentController::run(infrastructure::InfrastructureOrchestrator& orchestrator, const std::string& csv_path) {
    std::ofstream csv(csv_path);
    if (!csv.is_open()) {
        std::cerr << "Failed to open CSV for writing: " << csv_path << "\n";
        return;
    }

    // Headers
    csv << "Day,SolarGen_Wh,AnimalDensity,SoilMoisture,"
        << "PoolStorage_Wh,"
        << "IdentityReq_Wh,IdentityAlloc_Wh,IdentityConsumed_Wh,IdentityState,IdentityReliability,"
        << "SoilReq_Wh,SoilAlloc_Wh,SoilConsumed_Wh,SoilState,SoilReliability\n";

    for (int day = 1; day <= total_days_; ++day) {
        // 1. Generate Environment
        auto eco = generateDeterministicEcoInput(day);

        // 2. Step the Orchestrator
        orchestrator.dailyStep(eco);

        // 3. Extract metrics
        const auto& pool = orchestrator.getEnergyPool();
        const auto& identity = orchestrator.getIdentityNode();
        const auto& soil = orchestrator.getSoilNode();

        // 4. Log
        csv << day << ","
            << eco.solar_wh << "," << eco.animal_density << "," << eco.soil_moisture << ","
            << pool.currentStorage() << ","
            << identity.requestedEnergy() << "," << identity.allocatedEnergy() << "," << identity.consumedEnergy() << "," 
            << static_cast<int>(identity.currentState()) << "," << identity.reliabilityIndex() << ","
            << soil.requestedEnergy() << "," << soil.allocatedEnergy() << "," << soil.consumedEnergy() << "," 
            << static_cast<int>(soil.currentState()) << "," << soil.reliabilityIndex() << "\n";
    }

    std::cout << "Simulation complete. Data saved to " << csv_path << "\n";
}

} // namespace strata::domain::simulation
