#pragma once
#include "../infrastructure/InfrastructureTypes.hpp"

namespace strata::domain::identity {

struct IdentityEnergyProfile {
    double boot_wh_per_day{0.0};
    double idle_wh_per_day{0.0};
    double sensing_wh_per_event{0.0};
    double processing_wh_per_event{0.0};
    double communication_wh_per_event{0.0};
};

struct IdentityEnergyBreakdown {
    double boot_wh{0.0};
    double idle_wh{0.0};
    double sensing_wh{0.0};
    double processing_wh{0.0};
    double communication_wh{0.0};
    double total_wh{0.0};
};

class IdentityNode {
public:
    IdentityNode(double events_per_animal, double energy_per_event, double base_consumption);
    IdentityNode(double events_per_animal, const IdentityEnergyProfile& profile);

    // 1. Environmental Input
    void updateAnimalDensity(double density);
    
    // 2. Compute necessary energy based on load
    void computeEnergyDemand();
    
    // 3. Provide demand to the orchestrator
    double requestedEnergy() const;

    // 4. Receive allocation
    void receiveAllocatedEnergy(double energy);

    // 5. Update internal state based on received vs requested
    void updateOperationalState();

    strata::domain::infrastructure::OperationalState currentState() const;
    double reliabilityIndex() const; // [0.0, 1.0]
    double processedEvents() const;
    double totalDailyEvents() const;
    double allocatedEnergy() const;
    double consumedEnergy() const;
    const IdentityEnergyProfile& energyProfile() const;
    const IdentityEnergyBreakdown& requestedBreakdown() const;
    const IdentityEnergyBreakdown& consumedBreakdown() const;

private:
    // Fixed specs
    double events_por_animal_por_dia;
    IdentityEnergyProfile energy_profile_;
    double energia_por_evento_wh;
    double consumo_base_wh;

    // Daily varying inputs
    int numero_animais{0};
    double total_eventos_dia{0.0};

    // Energy tracking
    double energia_solicitada_wh{0.0};
    double energia_alocada_wh{0.0};
    double energia_efetivamente_utilizada_wh{0.0};
    IdentityEnergyBreakdown requested_breakdown_;
    IdentityEnergyBreakdown consumed_breakdown_;

    // Outputs
    double eventos_processados{0.0};
    double eventos_perdidos{0.0};
    double identity_reliability_index{0.0};
    
    strata::domain::infrastructure::OperationalState state{strata::domain::infrastructure::OperationalState::Full};
};

} // namespace strata::domain::identity
