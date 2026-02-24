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
