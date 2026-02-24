#include <gtest/gtest.h>

#include "application/Session.hpp"
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

fs::path uniqueTempRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() / ("strata_infra_report_test_" + std::to_string(stamp));
}

json loadJsonFromPath(const std::string& path) {
    std::ifstream in(path);
    EXPECT_TRUE(in.is_open());
    json report;
    in >> report;
    return report;
}

} // namespace

TEST(InfrastructureResilienceRunTest, WritesLatestInfrastructureArtifactsUnderProjectRoot) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    const std::string reportPath = session.runInfrastructureResilienceSimulation(30);
    ASSERT_FALSE(reportPath.empty());

    const fs::path latestJsonPath(reportPath);
    ASSERT_TRUE(fs::exists(latestJsonPath));

    std::ifstream in(latestJsonPath);
    ASSERT_TRUE(in.is_open());
    json report;
    in >> report;

    EXPECT_EQ(report.value("schemaVersion", ""), "infrastructure_resilience_report.v0.1");
    EXPECT_EQ(report.value("projectRoot", ""), projectRoot.string());
    EXPECT_EQ(report["runConfig"].value("days", 0), 30);

    const auto artifacts = report.value("artifacts", json::object());
    const fs::path csvLatest = artifacts.value("csvLatest", "");
    const fs::path jsonLatest = artifacts.value("jsonLatest", "");
    ASSERT_TRUE(fs::exists(csvLatest));
    ASSERT_TRUE(fs::exists(jsonLatest));

    const auto runConfig = report.value("runConfig", json::object());
    EXPECT_EQ(runConfig["identity"].value("energyModel", ""), "device_operational_profile_v0.1");
    EXPECT_EQ(runConfig["soil"].value("energyModel", ""), "device_operational_profile_v0.1");

    const auto finalState = report.value("finalState", json::object());
    EXPECT_TRUE(finalState.contains("poolStorageWh"));
    EXPECT_TRUE(finalState.contains("identity"));
    EXPECT_TRUE(finalState.contains("soil"));
    EXPECT_TRUE(finalState["identity"].contains("allocatedWh"));
    EXPECT_TRUE(finalState["soil"].contains("allocatedWh"));
    EXPECT_TRUE(finalState["identity"].contains("requestedBreakdownWh"));
    EXPECT_TRUE(finalState["identity"].contains("consumedBreakdownWh"));
    EXPECT_TRUE(finalState["soil"].contains("requestedBreakdownWh"));
    EXPECT_TRUE(finalState["soil"].contains("consumedBreakdownWh"));

    std::ifstream csvIn(csvLatest);
    ASSERT_TRUE(csvIn.is_open());
    std::string header;
    std::getline(csvIn, header);
    EXPECT_NE(header.find("IdentityAlloc_Wh"), std::string::npos);
    EXPECT_NE(header.find("IdentityConsumed_Wh"), std::string::npos);
    EXPECT_NE(header.find("SoilAlloc_Wh"), std::string::npos);
    EXPECT_NE(header.find("SoilConsumed_Wh"), std::string::npos);

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, AcceptsCustomFTConfiguration) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 15;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::SevereDrought;
    config.identityEventsPerAnimalPerDay = 12.0;
    config.identityProfile = {
        .boot_wh_per_day = 0.4,
        .idle_wh_per_day = 2.2,
        .sensing_wh_per_event = 0.7,
        .processing_wh_per_event = 0.3,
        .communication_wh_per_event = 0.1
    };
    config.ftNodeCount = 3;
    config.ftHardwareCostUsd = 120.5;
    config.ftComponentSelection = json::array({
        {{"role", "compute"}, {"name", "ESP32-S3"}},
        {{"role", "sensor"}, {"name", "IR Camera QVGA"}},
        {{"role", "radio"}, {"name", "LoRa SX1262"}}
    });
    config.trigger = "test_custom_ft_configuration";

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());

    std::ifstream in(reportPath);
    ASSERT_TRUE(in.is_open());
    json report;
    in >> report;

    const auto runConfig = report.value("runConfig", json::object());
    EXPECT_EQ(runConfig.value("days", 0), 15);
    EXPECT_EQ(runConfig["ecologicalScenario"].value("name", ""), "severe_drought_v0_1");
    EXPECT_EQ(runConfig["identity"].value("eventsPerAnimalPerDay", 0.0), 12.0);
    EXPECT_EQ(runConfig["ftHardware"].value("nodeCount", 0), 3);
    EXPECT_EQ(runConfig["ftHardware"].value("estimatedHardwareCostUSD", 0.0), 120.5);
    EXPECT_EQ(runConfig["ftHardware"]["components"].size(), 3u);
    EXPECT_EQ(report.value("trigger", ""), "test_custom_ft_configuration");

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, SevereDroughtReducesFinalPoolStorage) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig normalConfig;
    normalConfig.days = 180;
    normalConfig.ecologicalScenario = Application::InfrastructureEcologicalScenario::Normal;
    normalConfig.trigger = "test_normal_scenario";

    const std::string normalReportPath = session.runInfrastructureResilienceSimulation(normalConfig);
    ASSERT_FALSE(normalReportPath.empty());
    std::ifstream normalIn(normalReportPath);
    ASSERT_TRUE(normalIn.is_open());
    json normalReport;
    normalIn >> normalReport;
    const double normalPool = normalReport["finalState"].value("poolStorageWh", 0.0);

    Application::InfrastructureEvaluationConfig droughtConfig;
    droughtConfig.days = 180;
    droughtConfig.ecologicalScenario = Application::InfrastructureEcologicalScenario::SevereDrought;
    droughtConfig.trigger = "test_severe_drought_scenario";

    const std::string droughtReportPath = session.runInfrastructureResilienceSimulation(droughtConfig);
    ASSERT_FALSE(droughtReportPath.empty());
    std::ifstream droughtIn(droughtReportPath);
    ASSERT_TRUE(droughtIn.is_open());
    json droughtReport;
    droughtIn >> droughtReport;
    const double droughtPool = droughtReport["finalState"].value("poolStorageWh", 0.0);

    EXPECT_LT(droughtPool, normalPool);

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, SameSeedSameConfigSameHash) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 120;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::Normal;
    config.identityEventsPerAnimalPerDay = 18.0;
    config.trigger = "determinism_replay_test";
    config.determinism.seed = 123456u;
    config.determinism.tier = Application::DTO::DeterminismTier::T1_SeededDeterministic;
    config.determinism.entropySources = {};

    const std::string reportPathA = session.runInfrastructureResilienceSimulation(config);
    const std::string reportPathB = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPathA.empty());
    ASSERT_FALSE(reportPathB.empty());

    const json reportA = loadJsonFromPath(reportPathA);
    const json reportB = loadJsonFromPath(reportPathB);

    const std::string hashA = reportA.value("stateHash", "");
    const std::string hashB = reportB.value("stateHash", "");
    EXPECT_FALSE(hashA.empty());
    EXPECT_EQ(hashA.size(), 64u);
    EXPECT_EQ(hashA, hashB);

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, ReportContainsDeterminismMetadata) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 45;
    config.trigger = "determinism_metadata_test";
    config.determinism.seed = 20260224u;
    config.determinism.tier = Application::DTO::DeterminismTier::T1_SeededDeterministic;
    config.determinism.entropySources = {"none"};

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());
    const json report = loadJsonFromPath(reportPath);

    const auto determinism = report.value("determinism", json::object());
    EXPECT_EQ(determinism.value("seed", 0u), 20260224u);
    EXPECT_EQ(determinism.value("tier", ""), "T1_SeededDeterministic");
    EXPECT_TRUE(determinism.contains("entropySources"));

    const auto payload = report.value("deterministicStatePayload", json::object());
    EXPECT_EQ(payload.value("schemaVersion", 0), 1);
    EXPECT_TRUE(payload.contains("identity"));
    EXPECT_TRUE(payload.contains("soil"));
    EXPECT_TRUE(report.contains("stateHash"));
    EXPECT_EQ(report.value("stateHash", "").size(), 64u);

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, Tier1RequiresNonZeroSeed) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 15;
    config.determinism.seed = 0;
    config.determinism.tier = Application::DTO::DeterminismTier::T1_SeededDeterministic;

    EXPECT_THROW(
        session.runInfrastructureResilienceSimulation(config),
        std::invalid_argument
    );

    fs::remove_all(tempRoot);
}
