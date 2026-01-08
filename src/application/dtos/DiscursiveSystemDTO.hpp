#pragma once

#include "src/application/dtos/ObservationalCommonDTOs.hpp"
#include <string>
#include <vector>
#include <map>

namespace Application::DTO {

/**
 * @brief Read-only DTO for discursive system observations.
 */
struct DiscursiveSystemDTO {
    std::string id;
    std::vector<std::string> declaredProblems;
    std::vector<std::string> declaredActions;
    std::vector<std::string> allegedMechanisms;
    std::vector<std::string> expectedEffects;
    std::vector<SourceReferenceDTO> sourceReferences;
    TemporalContextDTO temporalContext;
    std::map<std::string, std::string> interpretationMetadata;
};

} // namespace Application::DTO
