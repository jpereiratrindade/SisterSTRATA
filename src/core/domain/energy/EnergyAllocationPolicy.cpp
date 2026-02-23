#include "EnergyAllocationPolicy.hpp"

namespace strata::domain::energy {

std::unordered_map<std::string, double> EqualitarianPolicy::allocate(
    double available_energy,
    const std::unordered_map<std::string, double>& requests) {
    
    std::unordered_map<std::string, double> allocations;
    if (requests.empty()) return allocations;

    // A simple v0.1 equalitarian policy that divides equally 
    // among all active requesters, capped by their request.
    // If we have less energy than total requested, everyone gets an equal slice 
    // of the available energy, up to what they asked for.
    // NOTE: A more robust implementation might iteratively redistribute leftovers.
    
    double equal_slice = available_energy / requests.size();

    double remaining_energy = available_energy;
    int unresolved_requests = requests.size();
    
    // First pass: give equal slice, or full request if they ask for less
    for (const auto& [id, req] : requests) {
        if (req <= equal_slice) {
            allocations[id] = req;
            remaining_energy -= req;
            unresolved_requests--;
        } else {
            allocations[id] = 0.0; // Mark for second pass
        }
    }

    // Second pass: distribute remaining fairly among those who asked for more
    if (unresolved_requests > 0) {
        double new_slice = remaining_energy / unresolved_requests;
        for (const auto& [id, req] : requests) {
            if (allocations[id] == 0.0 && req > 0) {
                // For v0.1, just give them the new equal slice. 
                // Any unused energy remains in the pool.
                allocations[id] = std::min(req, new_slice);
                remaining_energy -= allocations[id];
            }
        }
    }

    return allocations;
}

} // namespace strata::domain::energy
