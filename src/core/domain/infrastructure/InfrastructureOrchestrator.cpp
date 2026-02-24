#include "InfrastructureOrchestrator.hpp"

namespace strata::domain::infrastructure {

InfrastructureOrchestrator::InfrastructureOrchestrator(
    energy::EnergyPool pool,
    energy::EqualitarianPolicy policy,
    identity::IdentityNode identity_node,
    seto::SoilMonitorNode soil_node
) 
    : pool_(pool)
    , policy_(policy)
    , identity_(identity_node)
    , soil_(soil_node) 
{}

void InfrastructureOrchestrator::dailyStep(const EcologicalInput& eco) {
    // 1. Atualizar geração
    pool_.updateGeneration(eco.solar_wh);

    // 2. Nodes calculam demanda
    identity_.updateAnimalDensity(eco.animal_density);
    identity_.computeEnergyDemand();

    soil_.updateEnvironmentalFactors(eco.soil_moisture);
    soil_.computeEnergyDemand();

    // 3. Recolher demandas
    std::map<std::string, double> requests;
    requests["identity"] = identity_.requestedEnergy();
    requests["soil"] = soil_.requestedEnergy();

    // 4. Aplicar política
    auto allocations = policy_.allocate(pool_.currentStorage(), requests);

    // 5. Subtrair do pool
    for (const auto& [id, energy] : allocations) {
        pool_.subtractEnergy(energy);
    }

    // 6. Entregar energia
    identity_.receiveAllocatedEnergy(allocations["identity"]);
    soil_.receiveAllocatedEnergy(allocations["soil"]);

    // 7. Atualizar estados
    identity_.updateOperationalState();
    soil_.updateOperationalState();
}

} // namespace strata::domain::infrastructure
