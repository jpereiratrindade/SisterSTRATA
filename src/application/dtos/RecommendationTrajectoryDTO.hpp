#pragma once

#include "application/dtos/RecommendationSnapshotDTO.hpp"
#include <string>
#include <vector>
#include <map>

namespace Application::DTO {

/**
 * @brief Read-only DTO for an ordered recommendation trajectory.
 */
struct RecommendationTrajectoryDTO {
    std::string id;
    std::vector<RecommendationSnapshotDTO> snapshots;
    std::map<std::string, std::string> metadata;
};

} // namespace Application::DTO
