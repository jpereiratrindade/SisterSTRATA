#pragma once

#include <string>
#include <optional>

namespace Application::DTO {

/**
 * @brief Provenance metadata for observational DTOs.
 */
struct SourceReferenceDTO {
    std::string sourceType;
    std::string sourceId;
    std::string productionDate;
    std::optional<std::string> author;
};

/**
 * @brief Relative temporal context for observational DTOs.
 */
struct TemporalContextDTO {
    std::string category;
    std::string label;
};

} // namespace Application::DTO
