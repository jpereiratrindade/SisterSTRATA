#include <gtest/gtest.h>

#include "application/Session.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

fs::path uniqueTempRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() / ("strata_cross_context_test_" + std::to_string(stamp));
}

void writeJson(const fs::path& path, const json& payload) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path);
    EXPECT_TRUE(out.is_open());
    out << payload.dump(2);
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

TEST(CrossContextIsolationTest, InfrastructureRunCannotMutateEcologicalCoreState) {
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

TEST(CrossContextIsolationTest, InfrastructureRunPreservesCrossContextStateSnapshots) {
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

TEST(CrossContextIsolationTest, ValidObservationalIngestionThenInfrastructureRunPreservesCrossContextState) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";
    const fs::path inputsRoot = projectRoot / "inputs";
    const fs::path narrativesPath = inputsRoot / "narratives" / "NarrativeObservation.valid.json";
    const fs::path discursivePath = inputsRoot / "discursive" / "DiscursiveSystem.valid.json";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());
    session.getWorkspace().createWorld("Membrane Ingestion World", 11, 7);

    json narrativePayload;
    narrativePayload["history"] = json::array();
    narrativePayload["history"].push_back({
        {"id", "OBS-INGEST-001"},
        {"intent", {{"type", "describe"}}},
        {"source", {{"sourceId", "SRC-OBS-001"}, {"sourceType", "DOCUMENT"}}},
        {"temporalContext", {{"category", "CONTEMPORARY"}, {"label", "2026-Q1"}}},
        {"axes", json::array({
            {{"label", "Soil"}, {"description", "soil condition overview"}}
        })}
    });
    writeJson(narrativesPath, narrativePayload);

    json discursivePayload;
    discursivePayload["systems"] = json::array();
    discursivePayload["systems"].push_back({
        {"id", "DS-INGEST-001"},
        {"label", "Ingestion Discursive System"},
        {"declaredProblems", json::array({"water stress"})},
        {"declaredActions", json::array({"increase monitoring"})},
        {"allegedMechanisms", json::array({"seasonal drought"})},
        {"expectedEffects", json::array({"resilience reduction"})}
    });
    writeJson(discursivePath, discursivePayload);

    EXPECT_NO_THROW(session.ingestFromIW(narrativesPath.string()));
    EXPECT_NO_THROW(session.ingestFromIW(discursivePath.string()));

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

    ASSERT_FALSE(narrativeBefore.empty());
    ASSERT_FALSE(discursiveBefore.empty());

    Application::InfrastructureEvaluationConfig config;
    config.days = 35;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::Normal;
    config.trigger = "ingestion_then_infra_isolation_guard";

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

TEST(CrossContextIsolationTest, InvalidObservationalIngestionThenInfrastructureRunPreservesCrossContextState) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";
    const fs::path inputsRoot = projectRoot / "inputs";
    const fs::path invalidDiscursivePath = inputsRoot / "discursive" / "DiscursiveSystem.invalid.json";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());
    session.getWorkspace().createWorld("Membrane Attack Simulation World", 10, 6);

    json invalidDiscursivePayload;
    invalidDiscursivePayload["decisionDirective"] = "mutate_core_state";
    invalidDiscursivePayload["systems"] = json::array();
    invalidDiscursivePayload["systems"].push_back({
        {"id", "DS-ATTACK-001"},
        {"label", "Invalid runtime directive"},
        {"declaredProblems", json::array({"cross_context_mutation_attempt"})}
    });
    writeJson(invalidDiscursivePath, invalidDiscursivePayload);

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

    EXPECT_THROW(session.ingestFromIW(invalidDiscursivePath.string()), std::logic_error);

    const auto* worldAfterRejectedIngestion = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldAfterRejectedIngestion, nullptr);
    EXPECT_EQ(worldAfterRejectedIngestion->getName(), worldNameBefore);
    EXPECT_EQ(worldAfterRejectedIngestion->getResolution().width, worldWidthBefore);
    EXPECT_EQ(worldAfterRejectedIngestion->getResolution().height, worldHeightBefore);
    EXPECT_EQ(session.getWorkspace().getDatasets().size(), datasetsBefore);
    EXPECT_EQ(session.getTrajectory().getTimeSlices().size(), trajectorySlicesBefore);
    EXPECT_EQ(session.getNarrativeHistoryDTO().size(), narrativeBefore.size());
    EXPECT_EQ(session.getDiscursiveSystemDTOs().size(), discursiveBefore.size());
    expectRecommendationTrajectoryEqual(recommendationBefore, session.getRecommendationTrajectoryDTO());
    EXPECT_EQ(session.getNarrativeContextGraph().dump(), contextGraphBefore);

    Application::InfrastructureEvaluationConfig config;
    config.days = 30;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::SevereDrought;
    config.trigger = "invalid_ingestion_then_infra_isolation_guard";

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());

    const auto* worldAfterRun = session.getWorkspace().getWorld().get();
    ASSERT_NE(worldAfterRun, nullptr);
    const auto narrativeAfterRun = session.getNarrativeHistoryDTO();
    const auto discursiveAfterRun = session.getDiscursiveSystemDTOs();
    const auto recommendationAfterRun = session.getRecommendationTrajectoryDTO();
    const auto contextGraphAfterRun = session.getNarrativeContextGraph().dump();

    EXPECT_EQ(worldAfterRun->getName(), worldNameBefore);
    EXPECT_EQ(worldAfterRun->getResolution().width, worldWidthBefore);
    EXPECT_EQ(worldAfterRun->getResolution().height, worldHeightBefore);
    EXPECT_EQ(session.getWorkspace().getDatasets().size(), datasetsBefore);
    EXPECT_EQ(session.getTrajectory().getTimeSlices().size(), trajectorySlicesBefore);

    ASSERT_EQ(narrativeAfterRun.size(), narrativeBefore.size());
    ASSERT_EQ(discursiveAfterRun.size(), discursiveBefore.size());
    expectRecommendationTrajectoryEqual(recommendationBefore, recommendationAfterRun);
    EXPECT_EQ(contextGraphAfterRun, contextGraphBefore);

    fs::remove_all(tempRoot);
}
