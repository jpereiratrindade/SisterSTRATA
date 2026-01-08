#pragma once

#include "src/application/dtos/ObservationalCommonDTOs.hpp"
#include <string>
#include <vector>

namespace Application::DTO {

/**
 * @brief Read-only DTO for a single recommendation snapshot.
 */
struct RecommendationSnapshotDTO {
    std::string id;
    std::string recommendationText;
    std::vector<std::string> contextConditions;
    std::string intendedAction;
    std::string expectedOutcome;
    SourceReferenceDTO sourceReference;
    TemporalContextDTO temporalContext;
};

} // namespace Application::DTO
