#include <gtest/gtest.h>

#include "application/Session.hpp"
#include "application/services/MembraneContractEnforcement.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace {

fs::path uniqueTempRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() / ("strata_membrane_test_" + std::to_string(stamp));
}

Application::DTO::MembraneEnvelopeDTO validEnvelope() {
    return Application::DTO::MembraneEnvelopeDTO(
        "observational_evidence",
        false,
        std::nullopt,
        {
            "ScientificInstrumentationContext",
            "SoilElectricalObservability.latest.json",
            "2026-02-24T00:00:00Z"
        });
}

void expectSourceReferenceEqual(
    const Application::DTO::SourceReferenceDTO& lhs,
    const Application::DTO::SourceReferenceDTO& rhs) {
    EXPECT_EQ(lhs.sourceType, rhs.sourceType);
    EXPECT_EQ(lhs.sourceId, rhs.sourceId);
    EXPECT_EQ(lhs.productionDate, rhs.productionDate);
    EXPECT_EQ(lhs.author, rhs.author);
}

void expectTemporalContextEqual(
    const Application::DTO::TemporalContextDTO& lhs,
    const Application::DTO::TemporalContextDTO& rhs) {
    EXPECT_EQ(lhs.category, rhs.category);
    EXPECT_EQ(lhs.label, rhs.label);
}

void expectNarrativeStateEqual(
    const Application::DTO::NarrativeStateDTO& lhs,
    const Application::DTO::NarrativeStateDTO& rhs) {
    EXPECT_EQ(lhs.id, rhs.id);
    expectSourceReferenceEqual(lhs.source, rhs.source);
    expectTemporalContextEqual(lhs.temporalContext, rhs.temporalContext);
    EXPECT_EQ(lhs.intent.intentType, rhs.intent.intentType);

    ASSERT_EQ(lhs.axes.size(), rhs.axes.size());
    for (size_t i = 0; i < lhs.axes.size(); ++i) {
        EXPECT_EQ(lhs.axes[i].label, rhs.axes[i].label);
        EXPECT_EQ(lhs.axes[i].description, rhs.axes[i].description);
        EXPECT_EQ(lhs.axes[i].abstractionLevel, rhs.axes[i].abstractionLevel);
    }

    EXPECT_EQ(lhs.metadata, rhs.metadata);
    ASSERT_EQ(lhs.spatialScope.has_value(), rhs.spatialScope.has_value());
    if (lhs.spatialScope.has_value() && rhs.spatialScope.has_value()) {
        EXPECT_EQ(lhs.spatialScope->type, rhs.spatialScope->type);
        EXPECT_EQ(lhs.spatialScope->patchId, rhs.spatialScope->patchId);
        ASSERT_EQ(
            lhs.spatialScope->coordinates.has_value(),
            rhs.spatialScope->coordinates.has_value());
        if (lhs.spatialScope->coordinates.has_value() && rhs.spatialScope->coordinates.has_value()) {
            EXPECT_FLOAT_EQ(lhs.spatialScope->coordinates->x, rhs.spatialScope->coordinates->x);
            EXPECT_FLOAT_EQ(lhs.spatialScope->coordinates->y, rhs.spatialScope->coordinates->y);
            EXPECT_FLOAT_EQ(lhs.spatialScope->coordinates->z, rhs.spatialScope->coordinates->z);
        }
    }
}

void expectDiscursiveStateEqual(
    const Application::DTO::DiscursiveSystemDTO& lhs,
    const Application::DTO::DiscursiveSystemDTO& rhs) {
    EXPECT_EQ(lhs.id, rhs.id);
    EXPECT_EQ(lhs.declaredProblems, rhs.declaredProblems);
    EXPECT_EQ(lhs.declaredActions, rhs.declaredActions);
    EXPECT_EQ(lhs.allegedMechanisms, rhs.allegedMechanisms);
    EXPECT_EQ(lhs.expectedEffects, rhs.expectedEffects);
    ASSERT_EQ(lhs.sourceReferences.size(), rhs.sourceReferences.size());
    for (size_t i = 0; i < lhs.sourceReferences.size(); ++i) {
        expectSourceReferenceEqual(lhs.sourceReferences[i], rhs.sourceReferences[i]);
    }
    expectTemporalContextEqual(lhs.temporalContext, rhs.temporalContext);
    EXPECT_EQ(lhs.interpretationMetadata, rhs.interpretationMetadata);
}

void expectRecommendationSnapshotEqual(
    const Application::DTO::RecommendationSnapshotDTO& lhs,
    const Application::DTO::RecommendationSnapshotDTO& rhs) {
    EXPECT_EQ(lhs.id, rhs.id);
    EXPECT_EQ(lhs.recommendationText, rhs.recommendationText);
    EXPECT_EQ(lhs.contextConditions, rhs.contextConditions);
    EXPECT_EQ(lhs.intendedAction, rhs.intendedAction);
    EXPECT_EQ(lhs.expectedOutcome, rhs.expectedOutcome);
    expectSourceReferenceEqual(lhs.sourceReference, rhs.sourceReference);
    expectTemporalContextEqual(lhs.temporalContext, rhs.temporalContext);
}

void expectRecommendationTrajectoryEqual(
    const Application::DTO::RecommendationTrajectoryDTO& lhs,
    const Application::DTO::RecommendationTrajectoryDTO& rhs) {
    EXPECT_EQ(lhs.id, rhs.id);
    EXPECT_EQ(lhs.metadata, rhs.metadata);
    ASSERT_EQ(lhs.snapshots.size(), rhs.snapshots.size());
    for (size_t i = 0; i < lhs.snapshots.size(); ++i) {
        expectRecommendationSnapshotEqual(lhs.snapshots[i], rhs.snapshots[i]);
    }
}

} // namespace

TEST(MembraneContractEnforcementTest, RejectsDecisionDirectiveAcrossMembrane) {
    using namespace Application::Services::MembraneContract;

    ObservabilityToInfrastructureAdapter adapter;
    const Application::DTO::SoilElectricalObservabilityDTO observability(
        "dry_to_wet", 0.12, 8.0, "1kHz-10kHz");
    const Application::DTO::HardwareFeasibilityDTO feasibility(
        16, 12.5, 0.05, true, "none");

    const Application::DTO::MembraneEnvelopeDTO invalid(
        "observational_evidence",
        false,
        std::string("mutate_core_state"),
        {
            "ScientificInstrumentationContext",
            "HardwareFeasibility.latest.json",
            "2026-02-24T00:00:00Z"
        });

    EXPECT_THROW(adapter.ingest(observability, feasibility, invalid), ContractViolation);
    EXPECT_EQ(adapter.acceptedCount(), 0);
}

TEST(MembraneContractEnforcementTest, RejectsCausalSemanticsAcrossMembrane) {
    using namespace Application::Services::MembraneContract;

    InfrastructureToFourthDimensionAdapter adapter;
    const Application::DTO::InfrastructureResilienceEvidenceDTO evidence(
        "normal_deterministic_v0_1",
        5123.0,
        0.97,
        40.0,
        38.0,
        37.5,
        0.95,
        12.0,
        11.8,
        11.2);
    const Application::DTO::MembraneEnvelopeDTO invalid(
        "observational_evidence",
        true,
        std::nullopt,
        {
            "InfrastructureLayer",
            "InfrastructureResilience.latest.json",
            "2026-02-24T00:00:00Z"
        });

    EXPECT_THROW(adapter.ingest(evidence, invalid), ContractViolation);
    EXPECT_EQ(adapter.acceptedCount(), 0);
}

TEST(MembraneContractEnforcementTest, AcceptsCompliantReadOnlyMembranePayloads) {
    using namespace Application::Services::MembraneContract;

    ObservabilityToInfrastructureAdapter o2i;
    InfrastructureToFourthDimensionAdapter i2f;
    ObservabilityToFourthDimensionAdapter o2f;

    const auto envelope = validEnvelope();
    const Application::DTO::SoilElectricalObservabilityDTO observability(
        "dry_to_wet", 0.12, 8.0, "1kHz-10kHz");
    const Application::DTO::HardwareFeasibilityDTO feasibility(
        16, 12.5, 0.05, true, "none");
    const Application::DTO::InfrastructureResilienceEvidenceDTO infraEvidence(
        "normal_deterministic_v0_1",
        5123.0,
        0.97,
        40.0,
        38.0,
        37.5,
        0.95,
        12.0,
        11.8,
        11.2);
    const Application::DTO::ObservabilityVisibilityEvidenceDTO obsVisibility(
        "partially_detectable", 0.21);

    EXPECT_NO_THROW(o2i.ingest(observability, feasibility, envelope));
    EXPECT_NO_THROW(i2f.ingest(infraEvidence, envelope));
    EXPECT_NO_THROW(o2f.ingest(obsVisibility, envelope));

    EXPECT_EQ(o2i.acceptedCount(), 1);
    EXPECT_EQ(i2f.acceptedCount(), 1);
    EXPECT_EQ(o2f.acceptedCount(), 1);
}

TEST(MembraneContractEnforcementTest, InfrastructureRunCannotMutateEcologicalCoreState) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());
    session.getWorkspace().createWorld("Membrane Ecological World", 12, 8);

    const auto* worldBefore = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldBefore, nullptr);

    const std::string worldNameBefore = worldBefore->getName();
    const auto worldWidthBefore = worldBefore->getResolution().width;
    const auto worldHeightBefore = worldBefore->getResolution().height;
    const auto datasetsBefore = session.getWorkspace().getDatasets().size();
    const auto trajectorySlicesBefore = session.getTrajectory().getTimeSlices().size();

    Application::InfrastructureEvaluationConfig config;
    config.days = 20;
    config.trigger = "membrane_anti_feedback_test";

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());

    const auto* worldAfter = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldAfter, nullptr);

    EXPECT_EQ(worldAfter->getName(), worldNameBefore);
    EXPECT_EQ(worldAfter->getResolution().width, worldWidthBefore);
    EXPECT_EQ(worldAfter->getResolution().height, worldHeightBefore);
    EXPECT_EQ(session.getWorkspace().getDatasets().size(), datasetsBefore);
    EXPECT_EQ(session.getTrajectory().getTimeSlices().size(), trajectorySlicesBefore);

    fs::remove_all(tempRoot);
}

TEST(MembraneContractEnforcementTest, InfrastructureRunPreservesCrossContextStateSnapshots) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());
    session.getWorkspace().createWorld("Membrane CrossContext World", 16, 9);

    Application::DTO::NarrativeStateDTO narrative{};
    narrative.id = "NARR-001";
    narrative.source = {
        .sourceType = "field_report",
        .sourceId = "SRC-NARR-001",
        .productionDate = "2026-02-24T12:00:00Z",
        .author = std::string("observer")
    };
    narrative.temporalContext = {
        .category = "seasonal",
        .label = "wet-season-window"
    };
    narrative.intent = {.intentType = "describe_observation"};
    narrative.axes.push_back({
        .label = "soil_moisture",
        .description = "Moisture gradient increased",
        .abstractionLevel = "meso"
    });
    narrative.metadata = {
        {"phase", "baseline"},
        {"reviewed", "true"}
    };
    narrative.spatialScope = Application::DTO::SpatialScopeDTO{
        .type = "patch",
        .patchId = 7,
        .coordinates = Application::DTO::SpatialCoordinatesDTO{.x = 1.0f, .y = 2.0f, .z = 3.0f}
    };
    session.registerNarrativeStateDTO(narrative);

    Application::DTO::DiscursiveSystemDTO discursive{};
    discursive.id = "DISC-001";
    discursive.declaredProblems = {"water stress"};
    discursive.declaredActions = {"expand monitoring"};
    discursive.allegedMechanisms = {"seasonal drought"};
    discursive.expectedEffects = {"reduced resilience"};
    discursive.sourceReferences.push_back({
        .sourceType = "analysis_report",
        .sourceId = "SRC-DISC-001",
        .productionDate = "2026-02-24T12:10:00Z",
        .author = std::string("analyst")
    });
    discursive.temporalContext = {
        .category = "monthly",
        .label = "february"
    };
    discursive.interpretationMetadata = {
        {"confidence", "high"},
        {"scope", "infrastructure"}
    };
    session.registerDiscursiveSystemDTO(discursive);

    Application::DTO::RecommendationSnapshotDTO recommendation{};
    recommendation.id = "REC-001";
    recommendation.recommendationText = "Increase FT sampling cadence by 10%";
    recommendation.contextConditions = {"severe_drought", "energy_stress"};
    recommendation.intendedAction = "tune_sampling";
    recommendation.expectedOutcome = "improve anomaly detection";
    recommendation.sourceReference = {
        .sourceType = "recommendation_engine",
        .sourceId = "SRC-REC-001",
        .productionDate = "2026-02-24T12:20:00Z",
        .author = std::string("system")
    };
    recommendation.temporalContext = {
        .category = "immediate",
        .label = "current-cycle"
    };
    session.addRecommendationSnapshotDTO(recommendation);

    const auto* worldBefore = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldBefore, nullptr);

    const std::string worldNameBefore = worldBefore->getName();
    const auto worldWidthBefore = worldBefore->getResolution().width;
    const auto worldHeightBefore = worldBefore->getResolution().height;
    const auto datasetsBefore = session.getWorkspace().getDatasets().size();
    const auto trajectorySlicesBefore = session.getTrajectory().getTimeSlices().size();
    const auto narrativeBefore = session.getNarrativeHistoryDTO();
    const auto discursiveBefore = session.getDiscursiveSystemDTOs();
    const auto recommendationBefore = session.getRecommendationTrajectoryDTO();
    const auto contextGraphBefore = session.getNarrativeContextGraph().dump();

    Application::InfrastructureEvaluationConfig config;
    config.days = 45;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::SevereDrought;
    config.trigger = "cross_context_snapshot_guard";

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());

    const auto* worldAfter = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldAfter, nullptr);
    const auto narrativeAfter = session.getNarrativeHistoryDTO();
    const auto discursiveAfter = session.getDiscursiveSystemDTOs();
    const auto recommendationAfter = session.getRecommendationTrajectoryDTO();
    const auto contextGraphAfter = session.getNarrativeContextGraph().dump();

    EXPECT_EQ(worldAfter->getName(), worldNameBefore);
    EXPECT_EQ(worldAfter->getResolution().width, worldWidthBefore);
    EXPECT_EQ(worldAfter->getResolution().height, worldHeightBefore);
    EXPECT_EQ(session.getWorkspace().getDatasets().size(), datasetsBefore);
    EXPECT_EQ(session.getTrajectory().getTimeSlices().size(), trajectorySlicesBefore);

    ASSERT_EQ(narrativeAfter.size(), narrativeBefore.size());
    for (size_t i = 0; i < narrativeBefore.size(); ++i) {
        expectNarrativeStateEqual(narrativeBefore[i], narrativeAfter[i]);
    }

    ASSERT_EQ(discursiveAfter.size(), discursiveBefore.size());
    for (size_t i = 0; i < discursiveBefore.size(); ++i) {
        expectDiscursiveStateEqual(discursiveBefore[i], discursiveAfter[i]);
    }

    expectRecommendationTrajectoryEqual(recommendationBefore, recommendationAfter);
    EXPECT_EQ(contextGraphAfter, contextGraphBefore);

    fs::remove_all(tempRoot);
}

TEST(MembraneContractEnforcementTest, SessionIngestionRejectsInvalidMembraneEnvelopeAndPreservesState) {
    using json = nlohmann::json;

    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";
    const fs::path inputsDir = projectRoot / "inputs";
    const fs::path payloadPath = inputsDir / "DiscursiveSystem.invalid.json";

    fs::create_directories(inputsDir);

    json payload;
    payload["decisionDirective"] = "mutate_core_state";
    payload["systems"] = json::array();
    payload["systems"].push_back({
        {"id", "DS-INVALID-SESSION-1"},
        {"label", "Invalid"},
        {"declaredProblems", json::array({"invalid"})}
    });

    {
        std::ofstream out(payloadPath);
        ASSERT_TRUE(out.is_open());
        out << payload.dump(2);
    }

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    const auto discursiveBefore = session.getDiscursiveSystemCount();
    const auto narrativeBefore = session.getNarrativeHistoryDTO().size();
    const auto recommendationBefore = session.getRecommendationSnapshotCount();

    EXPECT_THROW(session.ingestFromIW(payloadPath.string()), std::logic_error);
    EXPECT_EQ(session.getDiscursiveSystemCount(), discursiveBefore);
    EXPECT_EQ(session.getNarrativeHistoryDTO().size(), narrativeBefore);
    EXPECT_EQ(session.getRecommendationSnapshotCount(), recommendationBefore);

    fs::remove_all(tempRoot);
}
