#include <gtest/gtest.h>

#include "application/services/ProjectPersistenceService.hpp"

#include <filesystem>
#include <fstream>

using Application::Services::ProjectPersistenceService;

class ProjectPersistenceServiceTest : public ::testing::Test {
protected:
    std::filesystem::path tempDir;
    std::unique_ptr<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem> narrative;
    std::unique_ptr<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository> discursive;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendation;
    std::unique_ptr<SisterSTRATA::Observational::Interpretation::InterpretationRepository> interpretation;

    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "sisterstrata_test_persistence";
        std::filesystem::create_directories(tempDir);

        narrative = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        discursive = std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>();
        interpretation = std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>();
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }
};

TEST_F(ProjectPersistenceServiceTest, AutoSaveCreatesFiles) {
    ProjectPersistenceService service(tempDir, *narrative, *discursive, recommendation, *interpretation);

    // Trigger all auto-saves — should not throw
    EXPECT_NO_THROW(service.autoSaveDiscursive());
    EXPECT_NO_THROW(service.autoSaveNarrative());
    EXPECT_NO_THROW(service.autoSaveRecommendation());
    EXPECT_NO_THROW(service.autoSaveInterpretation());

    // Files should be created
    EXPECT_TRUE(std::filesystem::exists(tempDir / "discursive_systems.json"));
    EXPECT_TRUE(std::filesystem::exists(tempDir / "narrative_history.json"));
    EXPECT_TRUE(std::filesystem::exists(tempDir / "recommendation_trajectory.json"));
    EXPECT_TRUE(std::filesystem::exists(tempDir / "interpretation_memory.json"));
}

TEST_F(ProjectPersistenceServiceTest, InitializePersistenceHandlesMissingFiles) {
    ProjectPersistenceService service(tempDir, *narrative, *discursive, recommendation, *interpretation);

    // Should not throw even when files don't exist
    EXPECT_NO_THROW(service.initializePersistence());
}

TEST_F(ProjectPersistenceServiceTest, InitializePersistenceHandlesCorruptFiles) {
    // Write invalid JSON content
    {
        std::ofstream out(tempDir / "discursive_systems.json");
        out << "NOT VALID JSON {{{";
    }

    ProjectPersistenceService service(tempDir, *narrative, *discursive, recommendation, *interpretation);

    // Should not throw even with corrupt files — graceful degradation
    EXPECT_NO_THROW(service.initializePersistence());
}

TEST_F(ProjectPersistenceServiceTest, SetProjectRootUpdatesPath) {
    ProjectPersistenceService service(tempDir, *narrative, *discursive, recommendation, *interpretation);

    auto newDir = tempDir / "subproject";
    std::filesystem::create_directories(newDir);
    service.setProjectRoot(newDir);

    service.autoSaveDiscursive();
    EXPECT_TRUE(std::filesystem::exists(newDir / "discursive_systems.json"));
    EXPECT_FALSE(std::filesystem::exists(tempDir / "discursive_systems.json"));
}
