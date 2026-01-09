#pragma once

#include "application/Session.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include "ui/components/FileSelector.hpp"
#include <string>
#include <optional>

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

    // -- Edit State --
    bool isEditing_ = false;
    std::string editingId_;

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
    std::optional<Application::DTO::SpatialScopeDTO> capturedScope_;

    UI::Components::FileSelector importSelector_{"Import Narrative JSON"};
    UI::Components::FileSelector exportSelector_{"Export Narrative JSON"};
    bool showImportDialog_ = false;
    bool showExportDialog_ = false;
    std::string lastImportPath_ = "assets/data/";
    std::string lastExportPath_ = "assets/data/";

    void drawIngestionForm();
    void drawObservationList();
    void loadIntoForm(const Application::DTO::NarrativeStateDTO& dto);

    // -- AI Analysis State --
    bool showAiModal_ = false;
    bool aiRequestPending_ = false;
    Application::DTO::Cognitive::InterpretationSnapshotDTO lastAiSnapshot_;
};

} // namespace UI::Panels
