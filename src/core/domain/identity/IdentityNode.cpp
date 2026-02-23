#include "IdentityNode.hpp"
#include <algorithm>
#include <cmath>

namespace strata::domain::identity {

IdentityNode::IdentityNode(double events_per_animal, double energy_per_event, double base_consumption)
    : IdentityNode(events_per_animal,
        IdentityEnergyProfile{
            .boot_wh_per_day = 0.0,
            .idle_wh_per_day = base_consumption,
            .sensing_wh_per_event = energy_per_event,
            .processing_wh_per_event = 0.0,
            .communication_wh_per_event = 0.0
        })
{}

IdentityNode::IdentityNode(double events_per_animal, const IdentityEnergyProfile& profile)
    : events_por_animal_por_dia(events_per_animal)
    , energy_profile_(profile)
    , energia_por_evento_wh(
        profile.sensing_wh_per_event +
        profile.processing_wh_per_event +
        profile.communication_wh_per_event)
    , consumo_base_wh(profile.boot_wh_per_day + profile.idle_wh_per_day)
{}

void IdentityNode::updateAnimalDensity(double density) {
    // Simple conversion from density to count for v0.1
    // e.g., if density is individuals/km2, we assume a 1km2 area
    numero_animais = static_cast<int>(std::round(density));
    total_eventos_dia = numero_animais * events_por_animal_por_dia;
}

void IdentityNode::computeEnergyDemand() {
    requested_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
    requested_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
    requested_breakdown_.sensing_wh = total_eventos_dia * energy_profile_.sensing_wh_per_event;
    requested_breakdown_.processing_wh = total_eventos_dia * energy_profile_.processing_wh_per_event;
    requested_breakdown_.communication_wh = total_eventos_dia * energy_profile_.communication_wh_per_event;
    requested_breakdown_.total_wh =
        requested_breakdown_.boot_wh +
        requested_breakdown_.idle_wh +
        requested_breakdown_.sensing_wh +
        requested_breakdown_.processing_wh +
        requested_breakdown_.communication_wh;

    energia_solicitada_wh = requested_breakdown_.total_wh;
}

double IdentityNode::requestedEnergy() const {
    return energia_solicitada_wh;
}

void IdentityNode::receiveAllocatedEnergy(double energy) {
    energia_alocada_wh = energy;
}

void IdentityNode::updateOperationalState() {
    using namespace strata::domain::infrastructure;

    // Default reset
    eventos_processados = 0.0;
    eventos_perdidos = total_eventos_dia;
    energia_efetivamente_utilizada_wh = 0.0;
    consumed_breakdown_ = IdentityEnergyBreakdown{};

    if (energia_alocada_wh == 0.0) {
        state = OperationalState::Suspended;
    } 
    else if (energia_alocada_wh < consumo_base_wh) {
        // Not enough to survive
        state = OperationalState::Suspended; 
        // We could model partial drain or battery death, but for now suspended.
    } 
    else if (energia_alocada_wh >= energia_solicitada_wh) {
        state = OperationalState::Full;
        eventos_processados = total_eventos_dia;
        eventos_perdidos = 0.0;
        energia_efetivamente_utilizada_wh = energia_solicitada_wh;
        consumed_breakdown_ = requested_breakdown_;
    } 
    else {
        // We have enough for base, but not all events
        double energy_for_events = energia_alocada_wh - consumo_base_wh;
        if (energy_for_events > 0.0) {
            state = OperationalState::Reduced;
            eventos_processados = std::floor(energy_for_events / energia_por_evento_wh);
            eventos_perdidos = total_eventos_dia - eventos_processados;
            consumed_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
            consumed_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
            consumed_breakdown_.sensing_wh = eventos_processados * energy_profile_.sensing_wh_per_event;
            consumed_breakdown_.processing_wh = eventos_processados * energy_profile_.processing_wh_per_event;
            consumed_breakdown_.communication_wh = eventos_processados * energy_profile_.communication_wh_per_event;
            consumed_breakdown_.total_wh =
                consumed_breakdown_.boot_wh +
                consumed_breakdown_.idle_wh +
                consumed_breakdown_.sensing_wh +
                consumed_breakdown_.processing_wh +
                consumed_breakdown_.communication_wh;
            energia_efetivamente_utilizada_wh = consumed_breakdown_.total_wh;
        } else {
            state = OperationalState::Survival;
            consumed_breakdown_.boot_wh = energy_profile_.boot_wh_per_day;
            consumed_breakdown_.idle_wh = energy_profile_.idle_wh_per_day;
            consumed_breakdown_.total_wh = consumed_breakdown_.boot_wh + consumed_breakdown_.idle_wh;
            energia_efetivamente_utilizada_wh = consumed_breakdown_.total_wh;
        }
    }

    // Reliability calculation
    if (total_eventos_dia > 0.0) {
        identity_reliability_index = eventos_processados / total_eventos_dia;
    } else {
        identity_reliability_index = 1.0; // Perfect if zero demand and standing by
    }
}

strata::domain::infrastructure::OperationalState IdentityNode::currentState() const {
    return state;
}

double IdentityNode::reliabilityIndex() const {
    return identity_reliability_index;
}

double IdentityNode::processedEvents() const { return eventos_processados; }
double IdentityNode::totalDailyEvents() const { return total_eventos_dia; }
double IdentityNode::allocatedEnergy() const { return energia_alocada_wh; }
double IdentityNode::consumedEnergy() const { return energia_efetivamente_utilizada_wh; }
const IdentityEnergyProfile& IdentityNode::energyProfile() const { return energy_profile_; }
const IdentityEnergyBreakdown& IdentityNode::requestedBreakdown() const { return requested_breakdown_; }
const IdentityEnergyBreakdown& IdentityNode::consumedBreakdown() const { return consumed_breakdown_; }

} // namespace strata::domain::identity
