#pragma once

#include "core/domain/land_use/TerritorialHypothesis.hpp"
#include "core/domain/territory/Territory.hpp"
#include "core/domain/territory/TerritorialCoherenceService.hpp"
#include "core/domain/shared/value_objects/CoherenceScore.hpp"
#include "core/domain/resilience/ResilienceReport.hpp"

#include <string>
#include <vector>
#include <future>
#include <atomic>

namespace UI::Panels {

/**
 * @brief Panel for inspecting and editing Territorial Hypotheses.
 * 
 * Allows the user to:
 * 1. Define typical Land Uses (LandUsePotential).
 * 2. Set Allocation Rules (Slope, Soil).
 * 3. Evaluate the hypothesis against the current Territory.
 */
class TerritorialHypothesisPanel {
public:
    TerritorialHypothesisPanel();
    
    /**
     * @brief Draws the panel.
     * @param open Reference to the visibility flag.
     */
    void draw(bool* open);

private:
    // Model state
    Core::Domain::LandUse::TerritorialHypothesis currentHypothesis_;
    Core::Domain::Territory::Territory currentTerritory_{"t-interactive", 100, 100}; // Placeholder for now, eventually linked to World3D
    
    // UI State
    std::string newLandUseId_ = "new_use";
    std::string newLandUseName_ = "New Land Use";
    int selectedHypothesisType_ = 0; // Index for combo box
    
    // Edit Modal State
    bool showEditModal_ = false;
    std::string editId_;
    std::string editName_;
    float editColor_[3] = {0.5f, 0.5f, 0.5f}; // RGB storage for ImGui ColorEdit
    
    // Rule Creation State
    int newRuleLandUseIndex_ = 0;
    float newRuleSlopeMin_ = 0.0f;
    float newRuleSlopeMax_ = 90.0f;
    int newRulePriority_ = 1;

    // Analysis Stats
    float minSlope_ = 0.0f;
    float maxSlope_ = 0.0f;
    float avgSlope_ = 0.0f;

    // Analysis Result Structure
    struct AnalysisResult {
        float minSlope;
        float maxSlope;
        float avgSlope;
        Core::Domain::Territory::Territory territory;
        Core::Domain::Shared::ValueObjects::CoherenceScore evaluation;
        std::vector<std::string> landUse; // Added for Trajectory Commit
    };

    // Async Analysis
    std::future<AnalysisResult> analysisFuture_;
    std::atomic<bool> isAnalyzing_{false};
    std::string analysisStatus_ = "Ready";

    // Evaluation Results
    std::string evaluationStatus_;
    float lastScore_ = 0.0f;
    std::string scoreDescription_;

    // Resilience State
    Core::Domain::Resilience::ResilienceReport lastResilienceReport_;

    void drawLandUseEditor();
    void drawAllocationRules();
    void drawEvaluationSection();
    /**
     * @brief Draws the Resilience Analysis tab.
     * Shows trajectory depth, overlap metrics, and system state assessment.
     */
    void drawResilienceSection(); // New
    void checkAsyncCompletion();
};

} // namespace UI::Panels
