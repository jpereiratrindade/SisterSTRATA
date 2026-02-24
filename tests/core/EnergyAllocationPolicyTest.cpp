#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

#include "core/domain/energy/EnergyAllocationPolicy.hpp"

namespace {

bool nearlyEqual(double lhs, double rhs, double eps = 1e-9) {
    return std::abs(lhs - rhs) <= eps;
}

void testEmptyRequests() {
    strata::domain::energy::EqualitarianPolicy policy;
    const std::map<std::string, double> requests;
    const auto allocations = policy.allocate(100.0, requests);
    assert(allocations.empty());
}

void testFairAllocationWithCap() {
    strata::domain::energy::EqualitarianPolicy policy;
    const std::map<std::string, double> requests{
        {"a", 1.0},
        {"b", 9.0},
        {"c", 9.0},
    };

    const auto allocations = policy.allocate(10.0, requests);
    assert(allocations.size() == 3);
    assert(nearlyEqual(allocations.at("a"), 1.0));
    assert(nearlyEqual(allocations.at("b"), 4.5));
    assert(nearlyEqual(allocations.at("c"), 4.5));
}

void testNonPositiveRequestsAndNonNegativeBudget() {
    strata::domain::energy::EqualitarianPolicy policy;
    const std::map<std::string, double> requests{
        {"sensor", 0.0},
        {"ft", 6.0},
        {"seto", 6.0},
    };

    const auto negativeEnergy = policy.allocate(-10.0, requests);
    assert(nearlyEqual(negativeEnergy.at("sensor"), 0.0));
    assert(nearlyEqual(negativeEnergy.at("ft"), 0.0));
    assert(nearlyEqual(negativeEnergy.at("seto"), 0.0));

    const auto regularEnergy = policy.allocate(6.0, requests);
    assert(nearlyEqual(regularEnergy.at("sensor"), 0.0));
    assert(nearlyEqual(regularEnergy.at("ft"), 3.0));
    assert(nearlyEqual(regularEnergy.at("seto"), 3.0));
}

void testEnoughEnergySatisfiesAllRequests() {
    strata::domain::energy::EqualitarianPolicy policy;
    const std::map<std::string, double> requests{
        {"a", 1.0},
        {"b", 2.0},
        {"c", 3.0},
    };

    const auto allocations = policy.allocate(12.0, requests);
    assert(nearlyEqual(allocations.at("a"), 1.0));
    assert(nearlyEqual(allocations.at("b"), 2.0));
    assert(nearlyEqual(allocations.at("c"), 3.0));
}

} // namespace

int main() {
    testEmptyRequests();
    testFairAllocationWithCap();
    testNonPositiveRequestsAndNonNegativeBudget();
    testEnoughEnergySatisfiesAllRequests();
    std::cout << "EnergyAllocationPolicy tests passed.\n";
    return 0;
}
