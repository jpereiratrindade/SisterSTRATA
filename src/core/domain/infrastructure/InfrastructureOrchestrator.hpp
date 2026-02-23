#pragma once
#include "InfrastructureTypes.hpp"
#include "../energy/EnergyPool.hpp"
#include "../energy/EnergyAllocationPolicy.hpp"
#include "../identity/IdentityNode.hpp"
#include "../soil/SoilMonitorNode.hpp"

namespace strata::domain::infrastructure {

class InfrastructureOrchestrator {
public:
    InfrastructureOrchestrator(
        energy::EnergyPool pool,
        energy::EqualitarianPolicy policy,
        identity::IdentityNode identity_node,
        soil::SoilMonitorNode soil_node
    );

    // The strict daily flow contract
    void dailyStep(const EcologicalInput& eco);

    // Accessors for metrics
    const identity::IdentityNode& getIdentityNode() const { return identity_; }
    const soil::SoilMonitorNode& getSoilNode() const { return soil_; }
    const energy::EnergyPool& getEnergyPool() const { return pool_; }

private:
    energy::EnergyPool pool_;
    energy::EqualitarianPolicy policy_;
    
    identity::IdentityNode identity_;
    soil::SoilMonitorNode soil_;
};

} // namespace strata::domain::infrastructure
