#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include "core/domain/land_use/TerritorialHypothesis.hpp"

using namespace Core::Domain::LandUse;

void test_hypothesis_creation() {
    std::cout << "Running test_hypothesis_creation..." << std::endl;
    
    // 1. Create Hypothesis (Type: Exploratory)
    TerritorialHypothesis hypothesis("hyp-001", "Transition 2030", HypothesisType::Exploratory);
    
    assert(hypothesis.getId() == "hyp-001");
    assert(hypothesis.getName() == "Transition 2030");
    assert(hypothesis.getType() == HypothesisType::Exploratory);
    
    // 2. Add Land Use Potentials
    LandUsePotential lu1("campestre", "Campestre", glm::vec3(0.1f));
    LandUsePotential lu2("florestal_natural", "Florestal Natural", glm::vec3(0.0f, 1.0f, 0.0f));
    
    hypothesis.addLandUseType(lu1);
    hypothesis.addLandUseType(lu2);
    
    assert(hypothesis.getLandUseTypes().size() == 2);
    assert(hypothesis.getLandUseTypes()[0].getName() == "Campestre");
    assert(hypothesis.getLandUseTypes()[1].getColor().y == 1.0f);
    
    // 3. Add Allocation Rules
    AllocationRule rule1;
    rule1.priority = 1;
    rule1.landUseId = "florestal_natural";
    rule1.method = AllocationMethod::GlobalConstraint;
    rule1.parameters["slope_min"] = "45"; // Degrees
    
    hypothesis.addAllocationRule(rule1);
    
    assert(hypothesis.getAllocationRules().size() == 1);
    assert(hypothesis.getAllocationRules()[0].priority == 1);
    assert(hypothesis.getAllocationRules()[0].parameters.at("slope_min") == "45");
    
    std::cout << "test_hypothesis_creation Passed!" << std::endl;
}

int main() {
    test_hypothesis_creation();
    std::cout << "All TerritorialHypothesis tests passed." << std::endl;
    return 0;
}
