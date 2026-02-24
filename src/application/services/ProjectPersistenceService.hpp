#pragma once

#include "observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "observational/discursive/aggregates/DiscursiveSystemRepository.hpp"
#include "observational/recommendation/aggregates/RecommendationTrajectory.hpp"
#include "observational/interpretation/aggregates/InterpretationRepository.hpp"
#include <filesystem>
#include <string>

namespace Application::Services {

/**
 * @brief Service responsible for auto-saving and loading
 *        observational data from the project directory.
 *
 * Extracted from Session.hpp to isolate persistence concerns.
 */
class ProjectPersistenceService {
public:
    ProjectPersistenceService(
        const std::filesystem::path& projectRoot,
        SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative,
        SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive,
        SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation,
        SisterSTRATA::Observational::Interpretation::InterpretationRepository& interpretation);

    /**
     * @brief Load all persisted data from the project root.
     */
    void initializePersistence();

    /**
     * @brief Update the project root path.
     */
    void setProjectRoot(const std::filesystem::path& root);

    void autoSaveDiscursive();
    void autoSaveNarrative();
    void autoSaveRecommendation();
    void autoSaveInterpretation();

private:
    std::filesystem::path projectRoot_;
    SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative_;
    SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation_;
    SisterSTRATA::Observational::Interpretation::InterpretationRepository& interpretation_;
};

} // namespace Application::Services
