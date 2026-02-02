#pragma once

#include <string>
#include <vector>

namespace Application::DTO::Soils {

struct ScorpanParamsDTO {
    float rainfall = 1500.0f;
    float temperature = 25.0f;
    float vegetationDensity = 0.5f;
    float ageFactor = 0.5f;
    int parentMaterial = 1; // Sedimentary
};

struct SiBCSFilterDTO {
    std::vector<int> allowedOrders;
    std::vector<int> allowedSuborders;
    std::vector<int> allowedGreatGroups;
    std::vector<int> allowedSubgroups;
    std::vector<int> allowedFamilies;
    std::vector<int> allowedSeries;
};

struct SoilOptionDTO {
    int code = 0;
    std::string label;
};

struct SoilLegendItemDTO {
    std::string label;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

} // namespace Application::DTO::Soils
