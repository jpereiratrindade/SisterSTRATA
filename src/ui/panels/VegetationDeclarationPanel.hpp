#pragma once

#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/vegetation/VegetationDeclarationService.hpp"
#include "core/domain/vegetation/VegetationMappingService.hpp"
#include "core/domain/vegetation/VegetationType.hpp"
#include <unordered_map>
#include <string>

namespace UI::Panels {

class VegetationDeclarationPanel {
public:
    VegetationDeclarationPanel();

    void draw(bool* open);

    // Access to the aggregate for rendering elsewhere if needed (or we render here?)
    const Core::Domain::Vegetation::VegetationSystemOriginal& getSystem() const { return system_; }

    const Core::Domain::Vegetation::VegetationMappingService::ScenarioResult* getLastScenarioResult() const {
        if (scenarioOutdated_) return nullptr;
        return &lastScenario_;
    }

private:
    Core::Domain::Vegetation::VegetationSystemOriginal system_;
    Core::Domain::Vegetation::VegetationDeclarationService service_;

    // UI State
    char idBuffer_[64] = "Hypothesis_01";
    int selectedType_ = 0; // 0=Campestre, 1=Florestal
    float minSlope_ = 0.0f;
    float maxSlope_ = 90.0f;
    float maxDistDrainage_ = 0.0f; // 0 = ignored

    // Cache to avoid FPS drop
    struct CachedStats {
        size_t matchVertices = 0;
        float coveragePercentage = 0.0f;
        float realizedPercentage = -1.0f; // -1 = not calculated
        bool outdated = true;
    };
    std::unordered_map<std::string, CachedStats> statsCache_;
    
    // Scenario Analysis
    bool scenarioOutdated_ = true;
    Core::Domain::Vegetation::VegetationMappingService::ScenarioResult lastScenario_;
};

} // namespace UI::Panels
