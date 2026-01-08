#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "src/observational/narrative/entities/NarrativeState.hpp"
#include "src/observational/narrative/value_objects/SourceReference.hpp"
#include "src/observational/narrative/value_objects/SemanticAxis.hpp"
#include "src/observational/narrative/value_objects/TemporalContext.hpp"
#include "src/observational/narrative/value_objects/ObservationIntent.hpp"

using namespace SisterSTRATA::Observational::Narrative;

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
        "Community resistance to infrastructure",
        SemanticAxis::AbstractionLevel::LOCAL
    );
    assert(axis.getLabel() == "Resistance");
    
    // Test TemporalContext (Updated Constructor for Category + Label separation)
    TemporalContext time(
        TemporalContext::RelativeTiming::PAST,
        "Before the flood"
    );
    assert(time.getCategory() == TemporalContext::RelativeTiming::PAST);
    assert(time.getLabel() == "Before the flood");

    // Test ObservationIntent
    ObservationIntent intent(ObservationIntent::IntentType::DESCRIPTIVE_RECORD);
    assert(intent.getType() == ObservationIntent::IntentType::DESCRIPTIVE_RECORD);
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
    
    ObservationIntent intent(ObservationIntent::IntentType::CONTEXTUALIZATION);

    SemanticAxis axis1("Myth", "Foundational myth", SemanticAxis::AbstractionLevel::REGIONAL);
    std::vector<SemanticAxis> axes = { axis1 };

    std::map<std::string, std::string> metadata;
    metadata["analyst"] = "JP";

    NarrativeState state(
        "STATE-001",
        source,
        time,
        intent,
        axes,
        metadata
    );

    assert(state.getId() == "STATE-001");
    // Ensure intent is correctly stored
    assert(state.getIntent().getType() == ObservationIntent::IntentType::CONTEXTUALIZATION);
}

void testSystemContainerBehavior() {
    NarrativeObservationSystem system;

    SourceReference source1(SourceReference::SourceType::FIELD_NOTE, "FN1", "2024");
    TemporalContext time1(TemporalContext::RelativeTiming::CONTEMPORARY, "Now");
    ObservationIntent intent(ObservationIntent::IntentType::DESCRIPTIVE_RECORD);

    NarrativeState state1(
        "S1",
        source1,
        time1,
        intent,
        {},
        {}
    );

    system.registerObservation(state1);

    const auto& history = system.getHistory();
    assert(history.size() == 1);
    assert(history[0].getId() == "S1");

    // Test 1: Duplicate ID Rejection
    bool caughtId = false;
    try {
        system.registerObservation(state1); 
    } catch (const std::invalid_argument& e) {
        caughtId = true;
    }
    assert(caughtId && "System should reject duplicate IDs");

    // Test 2: Strict Deduplication (Same Source + Same Time Label)
    // Create a new state with DIFFERENT ID but SAME Source & Time
    NarrativeState duplicateContentState(
        "S2", // Different ID
        source1,  // Same Source
        time1,    // Same Time Label
        intent,
        {},
        {}
    );

    bool caughtContent = false;
    try {
        system.registerObservation(duplicateContentState);
    } catch (const std::invalid_argument& e) {
        caughtContent = true;
        std::cout << "(Correctly caught duplicate content: " << e.what() << ") ";
    }
    assert(caughtContent && "System should reject duplicate Source+Time content even with different ID");
}

void testPersistence() {
    NarrativeObservationSystem system;
    
    // Setup Data
    SourceReference source(SourceReference::SourceType::INTERVIEW, "P-TEST", "2024-01-01");
    TemporalContext time(TemporalContext::RelativeTiming::CONTEMPORARY, "Test Time");
    ObservationIntent intent(ObservationIntent::IntentType::EXPLORATORY_HYPOTHESIS);
    SemanticAxis axis("Theme A", "Desc A", SemanticAxis::AbstractionLevel::LOCAL);
    
    std::map<std::string, std::string> meta;
    meta["author"] = "tester";

    NarrativeState state(
        "OBS-P1",
        source,
        time,
        intent,
        {axis},
        meta
    );

    system.registerObservation(state);

    // Save
    std::string filename = "test_persistence.json";
    system.serialize(filename);

    // Load into new system
    NarrativeObservationSystem system2;
    system2.deserialize(filename);

    const auto& history = system2.getHistory();
    assert(history.size() == 1);
    
    const auto& loaded = history[0];
    assert(loaded.getId() == "OBS-P1");
    assert(loaded.getSource().getSourceId() == "P-TEST");
    assert(loaded.getTemporalContext().getLabel() == "Test Time");
    assert(loaded.getIntent().getType() == ObservationIntent::IntentType::EXPLORATORY_HYPOTHESIS);
    assert(loaded.getAxes().size() == 1);
    assert(loaded.getAxes()[0].getLabel() == "Theme A");
    assert(loaded.getMetadata().at("author") == "tester");

    // Cleanup
    std::remove(filename.c_str());
}

int main() {
    runTest("ValueObjectsImmutability", testValueObjectsImmutability);
    runTest("NarrativeStateCreation", testNarrativeStateCreation);
    runTest("SystemContainerBehavior", testSystemContainerBehavior);
    runTest("Persistence", testPersistence);
    
    std::cout << "\nAll Narrative Observation tests passed." << std::endl;
    return 0;
}
