#include "src/ui/panels/NarrativePanel.hpp"
#include "imgui.h"
#include <vector>
#include <map>
#include "world3d/World3D.hpp" // Trusted include path

// Map Enums to Strings for UI
static const char* SOURCE_TYPES[] = { 
    "Interview", "Technical Report", "Historical Record", "Scientific Article", 
    "Institutional Document", "Media Article", "Field Note", "Other" 
};

static const char* TEMPORAL_CATEGORIES[] = {
    "Ancestral", "Past", "Recent Past", "Contemporary", "Future Vision", "Timeless", "Indeterminate"
};

static const char* INTENTS[] = {
    "Descriptive Record", "Exploratory Hypothesis", "Contextualization", "Methodological Note"
};

namespace UI::Panels {

void NarrativePanel::setSession(Application::Session* session) {
    session_ = session;
}

void NarrativePanel::draw(bool* open) {
    if (!open || !*open) return;

    // --- Picking Logic ---
    if (pickingMode_) {
        // Change cursor to indicate picking (optional, ImGui supports this)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        // Check for click
        if (ImGui::IsMouseClicked(0)) {
            // Check if we are hovering over any UI window
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
                ImVec2 mousePos = ImGui::GetMousePos();
                ImGuiIO& io = ImGui::GetIO();
                
                // Call World3D Pick
                int patchId = World3D::getPickIndex(mousePos.x, mousePos.y, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
                
                if (patchId > 0) { // Assuming 0 is invalid/background
                    capturedScope_ = SisterSTRATA::Observational::Narrative::SpatialScope(patchId);
                    pickingMode_ = false; // Auto-exit
                }
            }
        }
        
        // Allow exiting pick mode with Escape
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            pickingMode_ = false;
        }
    }

    // Basic Window Setup
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Narrative Observation Context", open)) {
        
        if (!session_) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: No Session Connected");
            ImGui::End();
            return;
        }

        // Two tabs: Ingestion and Visualization
        if (ImGui::BeginTabBar("NarrativeTabs")) {
            if (ImGui::BeginTabItem("Ingestion")) {
                drawIngestionForm();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Observation Log")) {
                drawObservationList();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

    }
    ImGui::End();
}

void NarrativePanel::drawIngestionForm() {
    ImGui::TextDisabled("Declarative Ingestion of Narrative Sources");
    ImGui::Separator();

    // 1. Source Reference
    ImGui::Text("Source Reference");
    ImGui::InputText("Source ID", inputSourceId_, IM_ARRAYSIZE(inputSourceId_));
    ImGui::InputText("Date/Year", inputDate_, IM_ARRAYSIZE(inputDate_));
    ImGui::Combo("Type", &inputSourceType_, SOURCE_TYPES, IM_ARRAYSIZE(SOURCE_TYPES));

    ImGui::Spacing();

    // 2. Temporal Context
    ImGui::Text("Temporal Context (Declared)");
    ImGui::Combo("Category", &inputTemporalCategory_, TEMPORAL_CATEGORIES, IM_ARRAYSIZE(TEMPORAL_CATEGORIES));
    ImGui::InputText("Label (e.g. 'Pós-barragem')", inputTemporalLabel_, IM_ARRAYSIZE(inputTemporalLabel_));

    ImGui::Spacing();

    // 3. Epistemological Intent
    ImGui::Text("Observation Intent");
    ImGui::Combo("Intent", &inputIntent_, INTENTS, IM_ARRAYSIZE(INTENTS));

    ImGui::Spacing();

    // 4. Content (Theme)
    ImGui::Text("Content / Theme");
    ImGui::InputText("Dominant Theme", inputTheme_, IM_ARRAYSIZE(inputTheme_));

    ImGui::Spacing();
    
    // 5. Spatial Anchoring
    ImGui::Text("Spatial Anchor");
    if (capturedScope_.has_value()) {
        auto& scope = capturedScope_.value();
        if (scope.getType() == SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType::PATCH_ID) {
            ImGui::TextColored(ImVec4(0,1,0,1), "Anchored to Patch: %d", scope.getPatchId().value_or(-1));
        } else {
             ImGui::TextColored(ImVec4(0,1,0,1), "Anchored (Other)");
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            capturedScope_ = std::nullopt;
        }
    } else {
        ImGui::TextDisabled("No spatial anchor set.");
        if (pickingMode_) {
            ImGui::TextColored(ImVec4(1,1,0,1), "CLICK ON TERRAIN TO ANCHOR...");
            ImGui::SameLine();
            if (ImGui::Button("Cancel Pick")) pickingMode_ = false;
        } else {
            if (ImGui::Button("Pick from Terrain")) {
                pickingMode_ = true;
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Register Observation", ImVec2(200, 0))) {
        // Construct Domain Objects
        using namespace SisterSTRATA::Observational::Narrative;

        // Source
        auto type = static_cast<SourceReference::SourceType>(inputSourceType_);
        SourceReference source(type, inputSourceId_, inputDate_);

        // Time
        auto timeCat = static_cast<TemporalContext::RelativeTiming>(inputTemporalCategory_);
        TemporalContext time(timeCat, inputTemporalLabel_);

        // Intent
        auto intentType = static_cast<ObservationIntent::IntentType>(inputIntent_);
        ObservationIntent intent(intentType);

        // Axis (Theme)
        std::vector<SemanticAxis> axes;
        if (strlen(inputTheme_) > 0) {
            axes.push_back(SemanticAxis(inputTheme_, inputTheme_, SemanticAxis::AbstractionLevel::LOCAL));
        }

        // Create State
        // Generate a simple ID (e.g. OBS-<Count>)
        size_t count = session_->getNarrativeSystem().getHistory().size() + 1;
        std::string id = "OBS-" + std::to_string(count);

        NarrativeState state(id, source, time, intent, axes, {}, capturedScope_);

        // Try Register
        try {
            session_->getNarrativeSystem().registerObservation(state);
            // Clear inputs on success
            // Keep source info for bulk entry
            capturedScope_ = std::nullopt; // Clear anchor
            ImGui::OpenPopup("Success");
        } catch (const std::exception& e) {
            ImGui::OpenPopup("Error");
        }
    }

    if (ImGui::BeginPopupModal("Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Observation registered successfully!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void NarrativePanel::drawObservationList() {
    auto& history = session_->getNarrativeSystem().getHistory();

    if (ImGui::BeginTable("ObservationsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Intent");
        ImGui::TableSetupColumn("Theme");
        ImGui::TableSetupColumn("Anchor");
        ImGui::TableHeadersRow();

        for (const auto& obs : history) {
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", obs.getId().c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s (%s)", obs.getSource().getSourceId().c_str(), obs.getSource().getProductionDate().c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", obs.getTemporalContext().getLabel().c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", obs.getIntent().toString().c_str());

            ImGui::TableSetColumnIndex(4);
            if (!obs.getAxes().empty()) {
                ImGui::Text("%s", obs.getAxes()[0].getLabel().c_str());
            } else {
                ImGui::Text("-");
            }
            
            ImGui::TableSetColumnIndex(5);
            if (obs.getSpatialScope().has_value()) {
                 auto& scope = obs.getSpatialScope().value();
                 if (scope.getType() == SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType::PATCH_ID) {
                     ImGui::Text("Patch %d", scope.getPatchId().value_or(0));
                 } else {
                     ImGui::Text("Yes");
                 }
            } else {
                ImGui::Text("-");
            }
        }
        ImGui::EndTable();
    }
}

} // namespace UI::Panels
