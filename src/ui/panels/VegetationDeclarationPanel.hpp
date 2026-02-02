#pragma once

#include "application/services/VegetationScenarioService.hpp"
#include "application/dtos/VegetationDTOs.hpp"
#include <unordered_map>
#include <string>

namespace UI::Panels {

/**
 * @brief UI Panel for declaring and managing vegetation hypotheses.
 * This panel reflects the declarative nature of the VegetationSystemOriginal,
 * allowing users to define scenarios and verify their potential coverage.
 */
class VegetationDeclarationPanel {
public:
    VegetationDeclarationPanel();

    /**
     * @brief Renders the ImGui interface for the panel.
     * @param open Pointer to a boolean controlling visibility.
     */
    void draw(bool* open);

    /**
     * @brief Returns the last global scenario resolution result.
     * @return Pointer to ScenarioResult, or nullptr if outdated.
     */
    const Application::DTO::Vegetation::ScenarioResultDTO* getLastScenarioResult() const {
        return service_.getLastScenarioResult();
    }

    /**
     * @brief Gets the last semantic classification map generated.
     * @return Const reference to a vector of semantic codes.
     */
    const std::vector<int>& getLastSemanticClassification() const { return service_.getLastSemanticClassification(); }

    /**
     * @brief Checks if the current view is a semantic (vector) classification.
     * @return true if semantic classification is active.
     */
    bool isSemanticClassificationActive() const { return service_.isSemanticClassificationActive(); }

    const std::vector<Application::DTO::Vegetation::ScenarioDTO>& getScenarioDTOs() const {
        return service_.getScenarioDTOs();
    }

    void saveScenarios(const std::string& path) { service_.saveScenarios(path); }
    void loadScenarios(const std::string& path) { service_.loadScenarios(path); }

    Application::Services::VegetationScenarioService& getService() { return service_; }
    const Application::Services::VegetationScenarioService& getService() const { return service_; }

private:
    Application::Services::VegetationScenarioService service_;

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
};

} // namespace UI::Panels
