#include <iostream>
#include <cassert>
#include "core/domain/territory/TerritorialCoherenceService.hpp"
#include "core/domain/territory/Territory.hpp"
#include "core/domain/land_use/TerritorialHypothesis.hpp"

using namespace Core::Domain::Territory;
using namespace Core::Domain::LandUse;

void test_coherence_evaluation() {
    std::cout << "Running test_coherence_evaluation..." << std::endl;

    // 1. Setup Territory
    Territory territory("t-001", 10, 10);
    // (In a real test, we would populate Hydro and Soils here)

    // 2. Setup Hypothesis
    TerritorialHypothesis hypothesis("h-001", "Test Hyp", HypothesisType::Exploratory);
    hypothesis.addLandUseType(LandUsePotential("campestre", "Campestre"));

    // 3. Evaluate
    Core::Domain::Shared::ValueObjects::CoherenceScore score = TerritorialCoherenceService::evaluate(territory, hypothesis);

    // 4. Verify
    std::cout << "Score Description: " << score.getDescription() << std::endl;
    // Since it's a mock, we expect the placeholder value
    assert(score.getValue() > 0.8f); 
    assert(score.isCoherent());

    std::cout << "test_coherence_evaluation Passed!" << std::endl;
}

int main() {
    test_coherence_evaluation();
    std::cout << "All TerritorialCoherenceService tests passed." << std::endl;
    return 0;
}
