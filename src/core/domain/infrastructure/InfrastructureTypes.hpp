#pragma once

namespace strata::domain::infrastructure {

struct EcologicalInput {
    double solar_wh{0.0};
    double animal_density{0.0};
    double soil_moisture{0.0}; // [0.0, 1.0]
};

// V0.1 basic states shared across resilience nodes
enum class OperationalState {
    Full,
    Reduced,
    Survival,
    Suspended,
    Recovering
};

} // namespace strata::domain::infrastructure
