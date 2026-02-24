#include "EnergyAllocationPolicy.hpp"
#include <algorithm>
#include <set>

namespace strata::domain::energy {

std::map<std::string, double> EqualitarianPolicy::allocate(
    double available_energy,
    const std::map<std::string, double>& requests) {

    std::map<std::string, double> allocations;
    if (requests.empty()) return allocations;

    // A simple v0.1 equalitarian policy that divides equally 
    // among all active requesters, capped by their request.
    // If we have less energy than total requested, everyone gets an equal slice 
    // of the available energy, up to what they asked for.
    // NOTE: A more robust implementation might iteratively redistribute leftovers.
    
    const double distributable_energy = std::max(0.0, available_energy);
    double equal_slice = distributable_energy / requests.size();

    double remaining_energy = distributable_energy;
    std::set<std::string> unresolved_ids;

    // First pass: give equal slice, or full request if they ask for less
    for (const auto& [id, req] : requests) {
        if (req <= 0.0) {
            allocations[id] = 0.0;
        } else if (req <= equal_slice) {
            allocations[id] = req;
            remaining_energy -= req;
        } else {
            allocations[id] = 0.0;
            unresolved_ids.insert(id);
        }
    }

    // Second pass: distribute remaining fairly among those who asked for more
    if (!unresolved_ids.empty()) {
        double new_slice = remaining_energy / unresolved_ids.size();
        for (const auto& [id, req] : requests) {
            if (unresolved_ids.find(id) != unresolved_ids.end()) {
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
