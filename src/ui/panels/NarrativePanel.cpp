#include "src/ui/panels/NarrativePanel.hpp"
#include "imgui.h"
#include "application/mappers/CognitiveMappers.hpp"
#include "application/mappers/CognitiveMappers.hpp"
#include "ui/components/InterpretationModal.hpp"
#include "ui/components/InterpretationHistory.hpp"
#include <vector>
#include <algorithm>
#include <map>
#include <cstring>
#include <filesystem>
#include "application/services/World3DService.hpp" // Trusted include path

// Map Enums to Strings for UI
static const char* SOURCE_TYPES[] = { 
    "Interview", "Technical Report", "Historical Record", "Scientific Article", 
    "Institutional Document", "Media Article", "Field Note", "Other" 
};
static const char* SOURCE_TYPE_VALUES[] = {
    "INTERVIEW", "TECHNICAL_REPORT", "HISTORICAL_RECORD", "SCIENTIFIC_ARTICLE",
    "INSTITUTIONAL_DOCUMENT", "MEDIA_ARTICLE", "FIELD_NOTE", "OTHER"
};

static const char* TEMPORAL_CATEGORIES[] = {
    "Ancestral", "Past", "Recent Past", "Contemporary", "Future Vision", "Timeless", "Indeterminate"
};
static const char* TEMPORAL_VALUES[] = {
    "ANCESTRAL", "PAST", "RECENT_PAST", "CONTEMPORARY", "FUTURE_VISION", "TIMELESS", "INDETERMINATE"
};

static const char* INTENTS[] = {
    "Descriptive Record", "Exploratory Hypothesis", "Contextualization", "Methodological Note"
};
static const char* INTENT_VALUES[] = {
    "DESCRIPTIVE_RECORD", "EXPLORATORY_HYPOTHESIS", "CONTEXTUALIZATION", "METHODOLOGICAL_NOTE"
};

static const char* intentLabel(const std::string& token) {
    if (token == "DESCRIPTIVE_RECORD") return "Descriptive Record";
    if (token == "EXPLORATORY_HYPOTHESIS") return "Exploratory Hypothesis";
    if (token == "CONTEXTUALIZATION") return "Contextualization";
    if (token == "METHODOLOGICAL_NOTE") return "Methodological Note";
    return token.c_str();
}

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
                int patchId = Application::Services::World3DService::getPickIndex(mousePos.x, mousePos.y, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
                
                if (patchId > 0) { // Assuming 0 is invalid/background
                    Application::DTO::SpatialScopeDTO scope;
                    scope.type = "PATCH_ID";
                    scope.patchId = patchId;
                    capturedScope_ = scope;
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

        if (ImGui::BeginTabBar("NarrativeTabs")) {
            
            if (ImGui::BeginTabItem("Observation Log")) {
                // Check for Deferred AI Results (Thread-safe UI update)
                {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    if (aiResultReady_) {
                        lastAiSnapshot_ = stagedAiSnapshot_; // Safe copy
                        ImGui::OpenPopup("AI Interpretation Result");
                        aiResultReady_ = false;
                    }
                }
                
                drawIngestionForm();
                ImGui::Separator();
                drawObservationList();

                // -- AI Modal Rendering --
                UI::Components::InterpretationModal::Draw("AI Interpretation Result", showAiModal_, lastAiSnapshot_, [this](const auto& snap) {
                    session_->saveInterpretationSnapshotDTO(snap);
                });
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Epistemic Memory")) {
                auto snapshots = session_->getInterpretationSnapshots();
                std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> filtered;
                
                // Filter for Narrative Context (Theme Analysis)
                std::copy_if(snapshots.begin(), snapshots.end(), std::back_inserter(filtered), [](const auto& s){
                    return s.intent == "theme_analysis";
                });
                
                std::reverse(filtered.begin(), filtered.end());
                UI::Components::InterpretationHistory::Draw(filtered);
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
        if (scope.type == "PATCH_ID" && scope.patchId.has_value()) {
            ImGui::TextColored(ImVec4(0,1,0,1), "Anchored to Patch: %d", scope.patchId.value());
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

    if (isEditing_) {
        ImGui::TextColored(ImVec4(1,1,0,1), "EDITING MODE: %s", editingId_.c_str());
        if (ImGui::Button("Cancel Edit")) {
            isEditing_ = false;
            editingId_.clear();
            // Clear inputs
            inputSourceId_[0] = '\0';
            inputTheme_[0] = '\0';
            inputTemporalLabel_[0] = '\0';
            capturedScope_ = std::nullopt;
        }
        ImGui::SameLine();
    }

    if (ImGui::Button(isEditing_ ? "Save Changes" : "Register Observation", ImVec2(200, 0))) {
        Application::DTO::NarrativeStateDTO dto;
        
        // ID Logic
        if (isEditing_) {
            dto.id = editingId_;
        } else {
            size_t count = session_->getNarrativeSystem().getHistory().size() + 1;
            dto.id = "OBS-" + std::to_string(count);
        }

        Application::DTO::SourceReferenceDTO source;
        source.sourceType = SOURCE_TYPE_VALUES[inputSourceType_];
        source.sourceId = inputSourceId_;
        source.productionDate = inputDate_;
        dto.source = source;

        dto.temporalContext = Application::DTO::TemporalContextDTO{
            TEMPORAL_VALUES[inputTemporalCategory_],
            inputTemporalLabel_
        };

        dto.intent = Application::DTO::ObservationIntentDTO{
            INTENT_VALUES[inputIntent_]
        };

        std::vector<Application::DTO::SemanticAxisDTO> axes;
        if (strlen(inputTheme_) > 0) {
            axes.push_back(Application::DTO::SemanticAxisDTO{
                inputTheme_,
                inputTheme_,
                "LOCAL"
            });
        }
        dto.axes = axes;
        dto.metadata = {};
        dto.spatialScope = capturedScope_;

        // Try Register/Update
        try {
            if (isEditing_) {
                session_->updateNarrativeStateDTO(editingId_, dto);
                ImGui::OpenPopup("NarrativeUpdateSuccess");
                isEditing_ = false;
                editingId_.clear();
            } else {
                session_->registerNarrativeStateDTO(dto);
                ImGui::OpenPopup("Success");
            }
            // Clear inputs on success
            inputSourceId_[0] = '\0'; // Optional: keep for bulk? Let's clear to be safe
            capturedScope_ = std::nullopt; 
        } catch (const std::exception& e) {
            ImGui::OpenPopup("Error");
        }
    }

    if (ImGui::BeginPopupModal("Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Observation registered successfully!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("NarrativeUpdateSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Observation updated successfully!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Import / Export");
    if (ImGui::Button("Import JSON")) {
        showImportDialog_ = true;
        importSelector_.Open(lastImportPath_);
    }
    ImGui::SameLine();
    if (ImGui::Button("Export JSON")) {
        showExportDialog_ = true;
        exportSelector_.Open(lastExportPath_);
    }

    std::string importResult;
    if (importSelector_.draw(&showImportDialog_, importResult, ".json")) {
        try {
            session_->loadNarrativeFromFile(importResult);
            std::filesystem::path selected(importResult);
            if (selected.has_parent_path()) {
                lastImportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("NarrativeImportSuccess");
        } catch (const std::exception& e) {
            importLastError_ = e.what();
            ImGui::OpenPopup("NarrativeImportError");
        }
        showImportDialog_ = false;
    }

    std::string exportResult;
    if (exportSelector_.draw(&showExportDialog_, exportResult, ".json", true)) {
        try {
            session_->saveNarrativeToFile(exportResult);
            std::filesystem::path selected(exportResult);
            if (selected.has_parent_path()) {
                lastExportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("NarrativeExportSuccess");
        } catch (const std::exception&) {
            ImGui::OpenPopup("NarrativeExportError");
        }
        showExportDialog_ = false;
    }

    if (ImGui::BeginPopupModal("NarrativeImportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Narrative observations imported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("NarrativeImportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to import narrative observations.");
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", importLastError_.c_str());
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("NarrativeExportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Narrative observations exported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("NarrativeExportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to export narrative observations.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void NarrativePanel::drawObservationList() {
    auto history = session_->getNarrativeHistoryDTO();

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "OBSERVATION LOG (NOC)");
    ImGui::SameLine(ImGui::GetWindowWidth() - 280);
    
    if (ImGui::Button(aiRequestPending_ ? "Waiting for Qwen..." : "Analyze Themes with Qwen", ImVec2(240, 0))) {
        auto history = session_->getNarrativeHistoryDTO();
        if (!history.empty()) {
            aiRequestPending_ = true;
            auto bundle = Application::Mappers::Cognitive::createBundle("theme_analysis", history);
            session_->requestAIInterpretation(bundle, 
                Application::Services::Cognitive::InterpretationMode::ThemeAnalysis,
                [this](const auto& snapshot) {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    stagedAiSnapshot_ = snapshot;
                    showAiModal_ = true;
                    aiRequestPending_ = false;
                    aiResultReady_ = true; // Signal the main thread to open the popup
                });
        }
    }

    if (ImGui::BeginTable("NarrativeLogTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Actions");
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
            ImGui::PushID(obs.id.c_str());
            if (ImGui::Button("Edit")) {
                loadIntoForm(obs);
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                session_->removeNarrativeStateDTO(obs.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", obs.id.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s (%s)", obs.source.sourceId.c_str(), obs.source.productionDate.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", obs.temporalContext.label.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", intentLabel(obs.intent.intentType));

            ImGui::TableSetColumnIndex(5);
            if (!obs.axes.empty()) {
                ImGui::Text("%s", obs.axes[0].label.c_str());
            } else {
                ImGui::Text("-");
            }
            
            ImGui::TableSetColumnIndex(6);
            if (obs.spatialScope.has_value()) {
                 auto& scope = obs.spatialScope.value();
                 if (scope.type == "PATCH_ID" && scope.patchId.has_value()) {
                     ImGui::Text("Patch %d", scope.patchId.value());
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

void NarrativePanel::loadIntoForm(const Application::DTO::NarrativeStateDTO& dto) {
    isEditing_ = true;
    editingId_ = dto.id;
    
    // Source
    strncpy(inputSourceId_, dto.source.sourceId.c_str(), sizeof(inputSourceId_) - 1);
    strncpy(inputDate_, dto.source.productionDate.c_str(), sizeof(inputDate_) - 1);
    
    for (int i = 0; i < IM_ARRAYSIZE(SOURCE_TYPE_VALUES); i++) {
        if (dto.source.sourceType == SOURCE_TYPE_VALUES[i]) {
            inputSourceType_ = i;
            break;
        }
    }

    // Time
    strncpy(inputTemporalLabel_, dto.temporalContext.label.c_str(), sizeof(inputTemporalLabel_) - 1);
    for (int i = 0; i < IM_ARRAYSIZE(TEMPORAL_VALUES); i++) {
        if (dto.temporalContext.category == TEMPORAL_VALUES[i]) {
            inputTemporalCategory_ = i;
            break;
        }
    }

    // Intent
    for (int i = 0; i < IM_ARRAYSIZE(INTENT_VALUES); i++) {
        if (dto.intent.intentType == INTENT_VALUES[i]) {
            inputIntent_ = i;
            break;
        }
    }

    // Theme (Axis 0)
    if (!dto.axes.empty()) {
        strncpy(inputTheme_, dto.axes[0].label.c_str(), sizeof(inputTheme_) - 1);
    } else {
        inputTheme_[0] = '\0';
    }

    // Spatial
    capturedScope_ = dto.spatialScope;
}

} // namespace UI::Panels
