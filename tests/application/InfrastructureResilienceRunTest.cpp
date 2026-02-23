#include <gtest/gtest.h>

#include "application/Session.hpp"
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

fs::path uniqueTempRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() / ("strata_infra_report_test_" + std::to_string(stamp));
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
    EXPECT_EQ(runConfig["identity"].value("eventsPerAnimalPerDay", 0.0), 12.0);
    EXPECT_EQ(runConfig["ftHardware"].value("nodeCount", 0), 3);
    EXPECT_EQ(runConfig["ftHardware"].value("estimatedHardwareCostUSD", 0.0), 120.5);
    EXPECT_EQ(runConfig["ftHardware"]["components"].size(), 3u);
    EXPECT_EQ(report.value("trigger", ""), "test_custom_ft_configuration");

    fs::remove_all(tempRoot);
}
