#include "SoilMonitorNode.hpp"

namespace strata::domain::seto {

SoilMonitorNode::SoilMonitorNode(double base_consumption, double max_dynamic_consumption)
    : SoilMonitorNode(
        SoilEnergyProfile{
            .boot_wh_per_day = 0.0,
            .idle_wh_per_day = base_consumption,
            .sensing_base_wh_per_day = 0.0,
            .communication_base_wh_per_day = 0.0,
            .dynamic_measurement_max_wh = max_dynamic_consumption
        })
{}

SoilMonitorNode::SoilMonitorNode(const SoilEnergyProfile& profile)
    : consumo_base_wh(
        profile.boot_wh_per_day +
        profile.idle_wh_per_day +
        profile.sensing_base_wh_per_day +
        profile.communication_base_wh_per_day)
    , consumo_dinamico_maximo_wh(profile.dynamic_measurement_max_wh)
    , energy_profile_(profile)
{}

void SoilMonitorNode::updateEnvironmentalFactors(double soil_moisture) {
    fator_umidade = soil_moisture;
}

void SoilMonitorNode::computeEnergyDemand() {
    // A simplified model: higher moisture = more frequent monitoring required = higher energy demand
    requested_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
    requested_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
    requested_breakdown_.sensing_base_wh = energy_profile_.sensing_base_wh_per_day;
    requested_breakdown_.communication_wh = energy_profile_.communication_base_wh_per_day;
    requested_breakdown_.sensing_dynamic_wh = consumo_dinamico_maximo_wh * fator_umidade;
    requested_breakdown_.total_wh =
        requested_breakdown_.boot_wh +
        requested_breakdown_.idle_wh +
        requested_breakdown_.sensing_base_wh +
        requested_breakdown_.communication_wh +
        requested_breakdown_.sensing_dynamic_wh;

    energia_solicitada_wh = requested_breakdown_.total_wh;
}

double SoilMonitorNode::requestedEnergy() const {
    return energia_solicitada_wh;
}

void SoilMonitorNode::receiveAllocatedEnergy(double energy) {
    energia_alocada_wh = energy;
}

void SoilMonitorNode::updateOperationalState() {
    using namespace strata::domain::infrastructure;

    energia_efetivamente_utilizada_wh = 0.0;
    consumed_breakdown_ = SoilEnergyBreakdown{};
    
    if (energia_alocada_wh == 0.0 || energia_alocada_wh < consumo_base_wh) {
        state = OperationalState::Suspended;
        monitoring_reliability_index = 0.0;
    } else if (energia_alocada_wh >= energia_solicitada_wh) {
        state = OperationalState::Full;
        consumed_breakdown_ = requested_breakdown_;
        energia_efetivamente_utilizada_wh = consumed_breakdown_.total_wh;
        monitoring_reliability_index = 1.0;
    } else {
        // Partial allocation
        double available_for_dynamic = energia_alocada_wh - consumo_base_wh;
        double ratio = 0.0;
        
        if (energia_solicitada_wh - consumo_base_wh > 0.0) {
             ratio = available_for_dynamic / (energia_solicitada_wh - consumo_base_wh);
        }

        if (ratio > 0.0) {
            state = OperationalState::Reduced;
            monitoring_reliability_index = ratio;
            consumed_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
            consumed_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
            consumed_breakdown_.sensing_base_wh = energy_profile_.sensing_base_wh_per_day;
            consumed_breakdown_.communication_wh = energy_profile_.communication_base_wh_per_day;
            consumed_breakdown_.sensing_dynamic_wh = available_for_dynamic;
            consumed_breakdown_.total_wh =
                consumed_breakdown_.boot_wh +
                consumed_breakdown_.idle_wh +
                consumed_breakdown_.sensing_base_wh +
                consumed_breakdown_.communication_wh +
                consumed_breakdown_.sensing_dynamic_wh;
            energia_efetivamente_utilizada_wh = consumed_breakdown_.total_wh;
        } else {
            state = OperationalState::Survival;
            monitoring_reliability_index = 0.0; // Just keeping alive, no data
            consumed_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
            consumed_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
            consumed_breakdown_.sensing_base_wh = energy_profile_.sensing_base_wh_per_day;
            consumed_breakdown_.communication_wh = energy_profile_.communication_base_wh_per_day;
            consumed_breakdown_.total_wh =
                consumed_breakdown_.boot_wh +
                consumed_breakdown_.idle_wh +
                consumed_breakdown_.sensing_base_wh +
                consumed_breakdown_.communication_wh;
            energia_efetivamente_utilizada_wh = consumed_breakdown_.total_wh;
        }
    }
}

strata::domain::infrastructure::OperationalState SoilMonitorNode::currentState() const {
    return state;
}

double SoilMonitorNode::reliabilityIndex() const {
    return monitoring_reliability_index;
}

double SoilMonitorNode::allocatedEnergy() const {
    return energia_alocada_wh;
}

double SoilMonitorNode::consumedEnergy() const {
    return energia_efetivamente_utilizada_wh;
}

const SoilEnergyProfile& SoilMonitorNode::energyProfile() const {
    return energy_profile_;
}

const SoilEnergyBreakdown& SoilMonitorNode::requestedBreakdown() const {
    return requested_breakdown_;
}

const SoilEnergyBreakdown& SoilMonitorNode::consumedBreakdown() const {
    return consumed_breakdown_;
}

} // namespace strata::domain::seto
