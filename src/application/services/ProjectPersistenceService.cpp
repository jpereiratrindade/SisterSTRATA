#include "application/services/ProjectPersistenceService.hpp"
#include "infrastructure/logging/Logger.hpp"

namespace Application::Services {

ProjectPersistenceService::ProjectPersistenceService(
    const std::filesystem::path& projectRoot,
    SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative,
    SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive,
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation,
    SisterSTRATA::Observational::Interpretation::InterpretationRepository& interpretation)
    : projectRoot_(projectRoot),
      narrative_(narrative),
      discursive_(discursive),
      recommendation_(recommendation),
      interpretation_(interpretation) {}

void ProjectPersistenceService::setProjectRoot(const std::filesystem::path& root) {
    projectRoot_ = root;
}

void ProjectPersistenceService::initializePersistence() {
    std::filesystem::path discursivePath = projectRoot_ / "discursive_systems.json";
    if (std::filesystem::exists(discursivePath)) {
        try {
            discursive_.deserialize(discursivePath.string());
        } catch (const std::exception& e) {
            LOG_ERROR("ProjectPersistence", std::string("Failed to load discursive: ") + e.what());
        } catch (...) {
            LOG_ERROR("ProjectPersistence", "Failed to load discursive (unknown error)");
        }
    }

    std::filesystem::path narrativePath = projectRoot_ / "narrative_history.json";
    if (std::filesystem::exists(narrativePath)) {
        try {
            narrative_.deserialize(narrativePath.string());
        } catch (const std::exception& e) {
            LOG_ERROR("ProjectPersistence", std::string("Failed to load narrative: ") + e.what());
        } catch (...) {
            LOG_ERROR("ProjectPersistence", "Failed to load narrative (unknown error)");
        }
    }

    std::filesystem::path recPath = projectRoot_ / "recommendation_trajectory.json";
    if (std::filesystem::exists(recPath)) {
        try {
            recommendation_.deserialize(recPath.string());
        } catch (const std::exception& e) {
            LOG_ERROR("ProjectPersistence", std::string("Failed to load recommendation: ") + e.what());
        } catch (...) {
            LOG_ERROR("ProjectPersistence", "Failed to load recommendation (unknown error)");
        }
    }

    std::filesystem::path interpPath = projectRoot_ / "interpretation_memory.json";
    if (std::filesystem::exists(interpPath)) {
        try {
            interpretation_.deserialize(interpPath.string());
        } catch (const std::exception& e) {
            LOG_ERROR("ProjectPersistence", std::string("Failed to load interpretation: ") + e.what());
        } catch (...) {
            LOG_ERROR("ProjectPersistence", "Failed to load interpretation (unknown error)");
        }
    }
}

void ProjectPersistenceService::autoSaveDiscursive() {
    try {
        discursive_.serialize((projectRoot_ / "discursive_systems.json").string());
    } catch (const std::exception& e) {
        LOG_ERROR("ProjectPersistence", std::string("Failed to save discursive: ") + e.what());
    }
}

void ProjectPersistenceService::autoSaveNarrative() {
    try {
        narrative_.serialize((projectRoot_ / "narrative_history.json").string());
    } catch (const std::exception& e) {
        LOG_ERROR("ProjectPersistence", std::string("Failed to save narrative: ") + e.what());
    }
}

void ProjectPersistenceService::autoSaveRecommendation() {
    try {
        recommendation_.serialize((projectRoot_ / "recommendation_trajectory.json").string());
    } catch (const std::exception& e) {
        LOG_ERROR("ProjectPersistence", std::string("Failed to save recommendation: ") + e.what());
    }
}

void ProjectPersistenceService::autoSaveInterpretation() {
    try {
        interpretation_.serialize((projectRoot_ / "interpretation_memory.json").string());
    } catch (const std::exception& e) {
        LOG_ERROR("ProjectPersistence", std::string("Failed to save interpretation: ") + e.what());
    }
}

} // namespace Application::Services
