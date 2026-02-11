#include <gtest/gtest.h>

#include "application/services/IWIngestionService.hpp"
#include "application/services/ProjectPersistenceService.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using Application::Services::IWIngestionService;
using Application::Services::ProjectPersistenceService;
using json = nlohmann::json;

class IWIngestionServiceTest : public ::testing::Test {
protected:
    std::filesystem::path tempDir;
    std::unique_ptr<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem> narrative;
    std::unique_ptr<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository> discursive;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendation;
    std::unique_ptr<SisterSTRATA::Observational::Interpretation::InterpretationRepository> interpretation;
    std::unique_ptr<ProjectPersistenceService> persistence;
    std::unique_ptr<IWIngestionService> service;

    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "sisterstrata_test_ingestion";
        std::filesystem::remove_all(tempDir);
        std::filesystem::create_directories(tempDir);

        narrative = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        discursive = std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>();
        interpretation = std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>();

        persistence = std::make_unique<ProjectPersistenceService>(
            tempDir, *narrative, *discursive, recommendation, *interpretation);

        service = std::make_unique<IWIngestionService>(
            *narrative, *discursive, recommendation, *persistence, tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    // Helper to write a JSON file
    void writeJson(const std::filesystem::path& path, const json& j) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path);
        out << j.dump(2);
    }
};

TEST_F(IWIngestionServiceTest, IngestStandaloneDiscursiveFile) {
    json payload;
    payload["systems"] = json::array();
    json system;
    system["id"] = "DS-TEST-1";
    system["label"] = "Test Discursive System";
    system["declaredProblems"] = json::array({"Soil degradation"});
    system["declaredActions"] = json::array({"Apply biochar"});
    system["allegedMechanisms"] = json::array({"Carbon sequestration"});
    system["expectedEffects"] = json::array({"Improved fertility"});
    payload["systems"].push_back(system);

    auto filePath = tempDir / "inputs" / "test_discursive.json";
    writeJson(filePath, payload);

    EXPECT_NO_THROW(service->ingestFromIW(filePath.string()));

    // Verify the discursive system was ingested
    EXPECT_GE(discursive->getSystems().size(), 1u);
}

TEST_F(IWIngestionServiceTest, IngestStandaloneNarrativeFile) {
    json payload;
    payload["history"] = json::array();
    json obs;
    obs["id"] = "OBS-TEST-1";
    obs["intent"] = {{"type", "describe"}};
    obs["source"] = {{"sourceId", "test-article"}, {"sourceType", "SCIENTIFIC_ARTICLE"}};
    obs["temporalContext"] = {{"category", "CONTEMPORARY"}, {"label", "2024"}};
    obs["axes"] = json::array();
    json axis;
    axis["label"] = "Solo";
    axis["description"] = "Estado do solo na área de estudo";
    obs["axes"].push_back(axis);
    payload["history"].push_back(obs);

    auto filePath = tempDir / "inputs" / "test_narrative.json";
    writeJson(filePath, payload);

    EXPECT_NO_THROW(service->ingestFromIW(filePath.string()));

    EXPECT_GE(narrative->getHistory().size(), 1u);
}

TEST_F(IWIngestionServiceTest, IngestFromNonExistentFileDoesNotCrash) {
    EXPECT_NO_THROW(service->ingestFromIW("/nonexistent/path/file.json"));
}

TEST_F(IWIngestionServiceTest, IngestFromNonExistentDirectoryDoesNotCrash) {
    EXPECT_NO_THROW(service->ingestFromIWDirectory("/nonexistent/directory"));
}

TEST_F(IWIngestionServiceTest, ScanForIngestionCreatesInputDirs) {
    // Remove any existing inputs dir
    std::filesystem::remove_all(tempDir / "inputs");

    service->scanForIngestion();

    // Should have created the standard directory structure
    EXPECT_TRUE(std::filesystem::exists(tempDir / "inputs" / "narratives"));
    EXPECT_TRUE(std::filesystem::exists(tempDir / "inputs" / "discursive"));
}

TEST_F(IWIngestionServiceTest, GetNarrativeHistoryDTOReturnsEmpty) {
    auto dtos = service->getNarrativeHistoryDTO();
    EXPECT_TRUE(dtos.empty());
}

TEST_F(IWIngestionServiceTest, IngestBundleDirectory) {
    // Create a minimal IW bundle
    auto bundleDir = tempDir / "inputs" / "test_bundle";

    json manifest;
    manifest["artifactId"] = "BUNDLE-TEST-1";
    writeJson(bundleDir / "Manifest.json", manifest);

    json discursivePayload;
    discursivePayload["systems"] = json::array();
    json sys;
    sys["id"] = "DS-BUNDLE-1";
    sys["label"] = "Bundle System";
    sys["declaredProblems"] = json::array({"Erosion"});
    sys["declaredActions"] = json::array({"Terracing"});
    sys["allegedMechanisms"] = json::array({"Water retention"});
    sys["expectedEffects"] = json::array({"Reduced runoff"});
    discursivePayload["systems"].push_back(sys);
    writeJson(bundleDir / "DiscursiveSystem.json", discursivePayload);

    json narrativePayload;
    narrativePayload["history"] = json::array();
    json obs;
    obs["id"] = "OBS-BUNDLE-1";
    obs["intent"] = {{"type", "describe"}};
    obs["source"] = {{"sourceId", "BUNDLE-TEST-1"}, {"sourceType", "DOCUMENT"}};
    obs["temporalContext"] = {{"category", "CONTEMPORARY"}, {"label", "2025"}};
    obs["axes"] = json::array({{{"label", "Hidrologia"}, {"description", "Balanço hídrico"}}});
    narrativePayload["history"].push_back(obs);
    writeJson(bundleDir / "NarrativeObservation.json", narrativePayload);

    EXPECT_NO_THROW(service->ingestFromIWDirectory((tempDir / "inputs").string()));

    EXPECT_GE(discursive->getSystems().size(), 1u);
    EXPECT_GE(narrative->getHistory().size(), 1u);
}
