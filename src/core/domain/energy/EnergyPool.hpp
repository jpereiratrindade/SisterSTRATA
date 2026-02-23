#pragma once

namespace strata::domain::energy {

struct EnergyPool {
    double total_capacity_wh{0.0};
    double current_storage_wh{0.0};
    double daily_generation_wh{0.0};

    // Constructor to set capacity and initial state
    EnergyPool(double capacity, double initial_storage) 
        : total_capacity_wh(capacity)
        , current_storage_wh(initial_storage) {}

    // Method to apply daily generation (saturates at capacity) Option B
    void updateGeneration(double solar_generation_wh) {
        daily_generation_wh = solar_generation_wh;
        current_storage_wh += daily_generation_wh;
        if (current_storage_wh > total_capacity_wh) {
            current_storage_wh = total_capacity_wh;
        }
    }

    void subtractEnergy(double amount) {
        current_storage_wh -= amount;
        if (current_storage_wh < 0.0) {
            current_storage_wh = 0.0;
        }
    }
    
    double currentStorage() const { return current_storage_wh; }
};

} // namespace strata::domain::energy
