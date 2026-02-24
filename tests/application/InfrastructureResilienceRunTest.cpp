#include <gtest/gtest.h>

#include "application/Session.hpp"
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
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

std::string sha256Hex(const std::string& payload) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char*>(payload.data()),
        payload.size(),
        digest);

    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char byte : digest) {
        out << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return out.str();
}

bool nearlyEqual(double lhs, double rhs, double eps = 1e-9) {
    return std::abs(lhs - rhs) <= eps;
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

TEST(InfrastructureResilienceRunTest, CrossContextEnergyInvariantsHoldForSevereDrought) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 120;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::SevereDrought;
    config.trigger = "cross_context_invariant_test";
    config.determinism.seed = 424242u;
    config.determinism.tier = Application::DTO::DeterminismTier::T1_SeededDeterministic;

    const std::string reportPath = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPath.empty());
    const json report = loadJsonFromPath(reportPath);

    const auto finalState = report.value("finalState", json::object());
    const auto identity = finalState.value("identity", json::object());
    const auto soil = finalState.value("soil", json::object());

    const double poolStorage = finalState.value("poolStorageWh", -1.0);
    const double idReq = identity.value("requestedWh", -1.0);
    const double idAlloc = identity.value("allocatedWh", -1.0);
    const double idCons = identity.value("consumedWh", -1.0);
    const double idReliability = identity.value("reliabilityIndex", -1.0);
    const double soilReq = soil.value("requestedWh", -1.0);
    const double soilAlloc = soil.value("allocatedWh", -1.0);
    const double soilCons = soil.value("consumedWh", -1.0);
    const double soilReliability = soil.value("reliabilityIndex", -1.0);

    EXPECT_GE(poolStorage, 0.0);

    EXPECT_GE(idReq, 0.0);
    EXPECT_GE(idAlloc, 0.0);
    EXPECT_GE(idCons, 0.0);
    EXPECT_LE(idAlloc, idReq + 1e-9);
    EXPECT_LE(idCons, idAlloc + 1e-9);
    EXPECT_GE(idReliability, 0.0);
    EXPECT_LE(idReliability, 1.0);

    EXPECT_GE(soilReq, 0.0);
    EXPECT_GE(soilAlloc, 0.0);
    EXPECT_GE(soilCons, 0.0);
    EXPECT_LE(soilAlloc, soilReq + 1e-9);
    EXPECT_LE(soilCons, soilAlloc + 1e-9);
    EXPECT_GE(soilReliability, 0.0);
    EXPECT_LE(soilReliability, 1.0);

    EXPECT_LE(idAlloc + soilAlloc, idReq + soilReq + 1e-9);

    const auto payload = report.value("deterministicStatePayload", json::object());
    ASSERT_FALSE(payload.is_null());
    const auto payloadIdentity = payload.value("identity", json::object());
    const auto payloadSoil = payload.value("soil", json::object());

    EXPECT_TRUE(nearlyEqual(payload.value("poolStorageWh", -2.0), poolStorage));
    EXPECT_TRUE(nearlyEqual(payloadIdentity.value("requestedWh", -2.0), idReq));
    EXPECT_TRUE(nearlyEqual(payloadIdentity.value("allocatedWh", -2.0), idAlloc));
    EXPECT_TRUE(nearlyEqual(payloadIdentity.value("consumedWh", -2.0), idCons));
    EXPECT_TRUE(nearlyEqual(payloadSoil.value("requestedWh", -2.0), soilReq));
    EXPECT_TRUE(nearlyEqual(payloadSoil.value("allocatedWh", -2.0), soilAlloc));
    EXPECT_TRUE(nearlyEqual(payloadSoil.value("consumedWh", -2.0), soilCons));

    fs::remove_all(tempRoot);
}

TEST(InfrastructureResilienceRunTest, StateHashMatchesCanonicalPayloadAndReplayPayloadIsIdentical) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path projectRoot = tempRoot / "project";

    Application::Session session;
    session.setProjectRoot(projectRoot.string());

    Application::InfrastructureEvaluationConfig config;
    config.days = 90;
    config.ecologicalScenario = Application::InfrastructureEcologicalScenario::Normal;
    config.identityEventsPerAnimalPerDay = 22.0;
    config.trigger = "determinism_hash_integrity_test";
    config.determinism.seed = 909090u;
    config.determinism.tier = Application::DTO::DeterminismTier::T1_SeededDeterministic;
    config.determinism.entropySources = {};

    const std::string reportPathA = session.runInfrastructureResilienceSimulation(config);
    const std::string reportPathB = session.runInfrastructureResilienceSimulation(config);
    ASSERT_FALSE(reportPathA.empty());
    ASSERT_FALSE(reportPathB.empty());

    const json reportA = loadJsonFromPath(reportPathA);
    const json reportB = loadJsonFromPath(reportPathB);

    const auto payloadA = reportA.value("deterministicStatePayload", json::object());
    const auto payloadB = reportB.value("deterministicStatePayload", json::object());
    const std::string canonicalPayloadA = payloadA.dump();
    const std::string canonicalPayloadB = payloadB.dump();

    EXPECT_EQ(canonicalPayloadA, canonicalPayloadB);

    const std::string expectedHashA = sha256Hex(canonicalPayloadA);
    const std::string expectedHashB = sha256Hex(canonicalPayloadB);
    EXPECT_EQ(reportA.value("stateHash", ""), expectedHashA);
    EXPECT_EQ(reportB.value("stateHash", ""), expectedHashB);
    EXPECT_EQ(expectedHashA, expectedHashB);
    std::cout << "DETERMINISM_STATE_HASH=" << expectedHashA << std::endl;

    fs::remove_all(tempRoot);
}
