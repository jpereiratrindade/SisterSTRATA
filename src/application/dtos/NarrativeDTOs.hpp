#pragma once

#include "application/dtos/ObservationalCommonDTOs.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace Application::DTO {

/**
 * @brief DTO for narrative observation intent.
 */
struct ObservationIntentDTO {
    std::string intentType;
};

/**
 * @brief DTO for narrative semantic axis.
 */
struct SemanticAxisDTO {
    std::string label;
    std::string description;
    std::string abstractionLevel;
};

/**
 * @brief DTO for spatial coordinates.
 */
struct SpatialCoordinatesDTO {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/**
 * @brief DTO for spatial scope anchor.
 */
struct SpatialScopeDTO {
    std::string type;
    std::optional<int> patchId;
    std::optional<SpatialCoordinatesDTO> coordinates;
};

/**
 * @brief Read-only DTO for a narrative observation state.
 */
struct NarrativeStateDTO {
    std::string id;
    SourceReferenceDTO source;
    TemporalContextDTO temporalContext;
    ObservationIntentDTO intent;
    std::vector<SemanticAxisDTO> axes;
    std::map<std::string, std::string> metadata;
    std::optional<SpatialScopeDTO> spatialScope;
};

} // namespace Application::DTO
