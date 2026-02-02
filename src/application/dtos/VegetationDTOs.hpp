#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Application::DTO::Vegetation {

struct DeclarationDTO {
    std::string id;
    int typeCode = 0;
    std::optional<float> minSlope;
    std::optional<float> maxSlope;
    std::optional<float> maxDistDrainage;
};

struct ComponentDTO {
    std::string typeLabel;
    std::optional<float> minSlope;
    std::optional<float> maxSlope;
    std::optional<float> maxDistanceToDrainage;
};

struct ScenarioDTO {
    std::string id;
    std::vector<ComponentDTO> components;
};

struct ScenarioResultDTO {
    std::vector<int> classification;
    std::vector<int> semanticCodes;
    std::vector<float> realizedPercentages;
};

struct CoverageResultDTO {
    size_t matchVertices = 0;
    float coveragePercentage = 0.0f;
    std::vector<bool> coverageMask;
};

} // namespace Application::DTO::Vegetation
