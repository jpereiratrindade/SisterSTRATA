#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "src/observational/narrative/entities/NarrativeState.hpp"
#include "src/observational/narrative/value_objects/SourceReference.hpp"
#include "src/observational/narrative/value_objects/SemanticAxis.hpp"
#include "src/observational/narrative/value_objects/TemporalContext.hpp"

using namespace SisterSTRATA::Observational::Narrative;

// Simple test runner helper
void runTest(const std::string& testName, void (*testFunc)()) {
    std::cout << "Running " << testName << "... ";
    try {
        testFunc();
        std::cout << "PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        exit(1);
    }
}

void testValueObjectsImmutability() {
    // Test SourceReference
    SourceReference source(
        SourceReference::SourceType::INTERVIEW,
        "INT-001",
        "2023-10-25"
    );
    assert(source.getSourceId() == "INT-001");
    assert(source.getProductionDate() == "2023-10-25");
    assert(source.getType() == SourceReference::SourceType::INTERVIEW);

    // Test SemanticAxis
    SemanticAxis axis(
        "Resistance",
        "Local resistance to dam construction",
        SemanticAxis::AbstractionLevel::LOCAL
    );
    assert(axis.getLabel() == "Resistance");
    
    // Test TemporalContext
    TemporalContext time(
        TemporalContext::RelativeTiming::PAST,
        "Before the flood"
    );
    assert(time.getTiming() == TemporalContext::RelativeTiming::PAST);
    assert(time.getDeclaredTimeLabel() == "Before the flood");
}

void testNarrativeStateCreation() {
    SourceReference source(
        SourceReference::SourceType::INSTITUTIONAL_DOCUMENT,
        "DOC-202",
        "1998"
    );

    TemporalContext time(
        TemporalContext::RelativeTiming::ANCESTRAL,
        "Ancient times"
    );

    SemanticAxis axis1("Myth", "Founding myth", SemanticAxis::AbstractionLevel::GLOBAL);
    std::vector<SemanticAxis> axes = { axis1 };

    std::map<std::string, std::string> metadata;
    metadata["analyst"] = "JP";

    NarrativeState state(
        "STATE-001",
        source,
        time,
        axes,
        metadata
    );

    assert(state.getId() == "STATE-001");
    assert(state.getAxes().size() == 1);
    assert(state.getMetadata().at("analyst") == "JP");
}

void testSystemContainerBehavior() {
    NarrativeObservationSystem system;

    auto state1 = std::make_shared<NarrativeState>(
        "S1",
        SourceReference(SourceReference::SourceType::FIELD_NOTE, "FN1", "2024"),
        TemporalContext(TemporalContext::RelativeTiming::CONTEMPORARY, "Now"),
        std::vector<SemanticAxis>{},
        std::map<std::string, std::string>{}
    );

    auto state2 = std::make_shared<NarrativeState>(
        "S2",
        SourceReference(SourceReference::SourceType::FIELD_NOTE, "FN2", "2024"),
        TemporalContext(TemporalContext::RelativeTiming::CONTEMPORARY, "Also Now"),
        std::vector<SemanticAxis>{},
        std::map<std::string, std::string>{}
    );

    system.registerObservation(state1);
    system.registerObservation(state2);

    const auto& history = system.getObservations();
    assert(history.size() == 2);
    assert(history[0]->getId() == "S1");
    assert(history[1]->getId() == "S2");

    // Test Duplicate Rejection
    bool caught = false;
    try {
        system.registerObservation(state1); // Duplicate ID "S1"
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught && "System should reject duplicate IDs");
}

int main() {
    runTest("ValueObjectsImmutability", testValueObjectsImmutability);
    runTest("NarrativeStateCreation", testNarrativeStateCreation);
    runTest("SystemContainerBehavior", testSystemContainerBehavior);
    
    std::cout << "\nAll Narrative Observation tests passed." << std::endl;
    return 0;
}
