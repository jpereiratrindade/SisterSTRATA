#pragma once
#include "../infrastructure/InfrastructureTypes.hpp"

namespace strata::domain::soil {

struct SoilEnergyProfile {
    double boot_wh_per_day{0.0};
    double idle_wh_per_day{0.0};
    double sensing_base_wh_per_day{0.0};
    double communication_base_wh_per_day{0.0};
    double dynamic_measurement_max_wh{0.0};
};

struct SoilEnergyBreakdown {
    double boot_wh{0.0};
    double idle_wh{0.0};
    double sensing_base_wh{0.0};
    double communication_wh{0.0};
    double sensing_dynamic_wh{0.0};
    double total_wh{0.0};
};

// Simplified SETO node for v0.1 Sprint 1
// Static demand or simple function of moisture to create energy competition
class SoilMonitorNode {
public:
    SoilMonitorNode(double base_consumption, double max_dynamic_consumption);
    SoilMonitorNode(const SoilEnergyProfile& profile);

    void updateEnvironmentalFactors(double soil_moisture);
    void computeEnergyDemand();
    double requestedEnergy() const;
    void receiveAllocatedEnergy(double energy);
    void updateOperationalState();

    strata::domain::infrastructure::OperationalState currentState() const;
    double reliabilityIndex() const; // [0.0, 1.0]
    double allocatedEnergy() const;
    double consumedEnergy() const;
    const SoilEnergyProfile& energyProfile() const;
    const SoilEnergyBreakdown& requestedBreakdown() const;
    const SoilEnergyBreakdown& consumedBreakdown() const;

private:
    double consumo_base_wh;
    double consumo_dinamico_maximo_wh;
    SoilEnergyProfile energy_profile_;

    double fator_umidade{0.0};
    
    double energia_solicitada_wh{0.0};
    double energia_alocada_wh{0.0};
    double energia_efetivamente_utilizada_wh{0.0};
    SoilEnergyBreakdown requested_breakdown_;
    SoilEnergyBreakdown consumed_breakdown_;

    double monitoring_reliability_index{0.0};
    strata::domain::infrastructure::OperationalState state{strata::domain::infrastructure::OperationalState::Full};
};

} // namespace strata::domain::soil
