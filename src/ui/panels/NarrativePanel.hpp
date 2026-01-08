#pragma once

#include "application/Session.hpp"
#include <string>
#include <optional>
#include "src/observational/narrative/value_objects/SpatialScope.hpp"

namespace UI::Panels {

/**
 * @brief UI Panel for the Narrative Observation Context.
 * 
 * Provides a "Passive" interface to:
 * 1. Manually ingest new narrative observations (Descriptive, Exploratory, etc.).
 * 2. Visualise the history of registered observations.
 * 
 * DESIGN:
 * - Observes Application::Session to access the NarrativeObservationSystem.
 * - Does NOT perform semantic validation (that's the Domain's job, or lack thereof).
 */
class NarrativePanel {
public:
    NarrativePanel() = default;

    /**
     * @brief Injects the Application Session dependency.
     * @param session Pointer to the active session.
     */
    void setSession(Application::Session* session);

    /**
     * @brief Renders the Narrative Panel using ImGui.
     * @param open Pointer to boolean controlling window visibility.
     */
    void draw(bool* open);

private:
    Application::Session* session_ = nullptr;

    // -- Form State --
    char inputSourceId_[64] = "";
    char inputDate_[64] = "";
    int inputSourceType_ = 0; // Index for combo
    
    int inputTemporalCategory_ = 3; // Default to Contemporary
    char inputTemporalLabel_[128] = "";
    
    int inputIntent_ = 0; // Default to Descriptive
    
    char inputContent_[1024] = ""; // Content/Theme/Description (mapped to SemanticAxis for now?)
    // Actually simplicity: We might just map content to a "Theme" axis for now, or just metadata
    // Plan said "Content/Theme". Let's treat it as a Semantic Axis label for simplicity.
    char inputTheme_[128] = ""; 

    // -- Spatial State --
    bool pickingMode_ = false;
    std::optional<SisterSTRATA::Observational::Narrative::SpatialScope> capturedScope_;

    void drawIngestionForm();
    void drawObservationList();
};

} // namespace UI::Panels
