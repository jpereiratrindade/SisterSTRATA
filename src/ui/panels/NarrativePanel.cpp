#include "src/ui/panels/NarrativePanel.hpp"
#include "imgui.h"
#include "application/mappers/CognitiveMappers.hpp"
#include "ui/components/InterpretationModal.hpp"
#include "ui/components/InterpretationHistory.hpp"
#include <vector>
#include <algorithm>
#include <map>
#include <cstring>
#include <filesystem>
#include <cmath>
#include <unordered_map>
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

static std::string metadataValueByPriority(const std::map<std::string, std::string>& metadata,
                                           const std::vector<std::string>& keys) {
    for (const auto& key : keys) {
        auto it = metadata.find(key);
        if (it != metadata.end()) {
            return it->second;
        }
    }
    return "";
}

struct NarrativeGraphNodeUI {
    std::string id;
    std::string label;
    std::string dominantDimension;
    std::string dominantIntent;
    int narrativeCount = 0;
    std::vector<std::string> topTokens;
    std::vector<std::string> artifactIds;
    std::vector<std::string> observationIds;
    ImVec2 position {};
    float radius = 12.0f;
};

struct NarrativeGraphEdgeUI {
    int source = -1;
    int target = -1;
    float similarity = 0.0f;
    float distance = 1.0f;
    int sharedTokens = 0;
};

static ImU32 colorForDimension(const std::string& dimension) {
    if (dimension == "ecological") return IM_COL32(90, 190, 110, 220);
    if (dimension == "productive") return IM_COL32(225, 170, 70, 220);
    if (dimension == "social") return IM_COL32(95, 145, 225, 220);
    return IM_COL32(170, 170, 185, 210);
}

static const char* labelForDimension(const std::string& dimension) {
    if (dimension == "ecological") return "Ecological";
    if (dimension == "productive") return "Productive";
    if (dimension == "social") return "Social";
    return "Mixed";
}

static void drawDimensionLegendItem(const char* id, ImU32 color, const char* label) {
    const ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
    ImGui::ColorButton(id, c, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(14, 14));
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
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

            if (ImGui::BeginTabItem("Context Graph")) {
                drawNarrativeGraph();
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
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputText("Source ID", inputSourceId_, IM_ARRAYSIZE(inputSourceId_));
    ImGui::InputText("Date/Year", inputDate_, IM_ARRAYSIZE(inputDate_));
    ImGui::Combo("Type", &inputSourceType_, SOURCE_TYPES, IM_ARRAYSIZE(SOURCE_TYPES));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // 2. Temporal Context
    ImGui::Text("Temporal Context (Declared)");
    ImGui::PushItemWidth(-1.0f);
    ImGui::Combo("Category", &inputTemporalCategory_, TEMPORAL_CATEGORIES, IM_ARRAYSIZE(TEMPORAL_CATEGORIES));
    ImGui::InputText("Label (e.g. 'Pós-barragem')", inputTemporalLabel_, IM_ARRAYSIZE(inputTemporalLabel_));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // 3. Epistemological Intent
    ImGui::Text("Observation Intent");
    ImGui::PushItemWidth(-1.0f);
    ImGui::Combo("Intent", &inputIntent_, INTENTS, IM_ARRAYSIZE(INTENTS));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // 4. Content (Theme)
    ImGui::Text("Content / Theme");
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputText("Dominant Theme", inputTheme_, IM_ARRAYSIZE(inputTheme_));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // 5. Evidence (optional)
    ImGui::Text("Evidence (optional)");
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputTextMultiline("Evidence Snippet##NarrativeEvidence", inputEvidenceSnippet_, IM_ARRAYSIZE(inputEvidenceSnippet_), ImVec2(-1, 60));
    ImGui::InputText("Source Section##NarrativeSourceSection", inputSourceSection_, IM_ARRAYSIZE(inputSourceSection_));
    ImGui::InputText("Page Range##NarrativePageRange", inputPageRange_, IM_ARRAYSIZE(inputPageRange_));
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Text("Metadata (optional)");
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputText("Metadata Key##NarrativeMetaKey", inputMetadataKey_, IM_ARRAYSIZE(inputMetadataKey_));
    ImGui::InputText("Metadata Value##NarrativeMetaValue", inputMetadataValue_, IM_ARRAYSIZE(inputMetadataValue_));
    ImGui::PopItemWidth();
    if (ImGui::Button("Add Metadata##Narrative")) {
        if (strlen(inputMetadataKey_) > 0) {
            metadata_[inputMetadataKey_] = inputMetadataValue_;
            inputMetadataKey_[0] = '\0';
            inputMetadataValue_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Metadata##Narrative")) {
        metadata_.clear();
    }

    if (!metadata_.empty()) {
        ImGui::TextDisabled("Current metadata:");
        for (const auto& [key, value] : metadata_) {
            ImGui::BulletText("%s: %s", key.c_str(), value.c_str());
        }
    }

    ImGui::Spacing();

    // 6. Spatial Anchoring
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
            inputEvidenceSnippet_[0] = '\0';
            inputSourceSection_[0] = '\0';
            inputPageRange_[0] = '\0';
            inputMetadataKey_[0] = '\0';
            inputMetadataValue_[0] = '\0';
            metadata_.clear();
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

        std::map<std::string, std::string> normalizedMetadata;
        for (const auto& [key, value] : metadata_) {
            if (key.rfind("iw.", 0) == 0) normalizedMetadata[key] = value;
            else normalizedMetadata["iw." + key] = value;
        }
        if (strlen(inputEvidenceSnippet_) > 0) normalizedMetadata["iw.evidenceSnippet"] = inputEvidenceSnippet_;
        if (strlen(inputSourceSection_) > 0) normalizedMetadata["iw.sourceSection"] = inputSourceSection_;
        if (strlen(inputPageRange_) > 0) normalizedMetadata["iw.pageRange"] = inputPageRange_;
        dto.metadata = std::move(normalizedMetadata);
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
            inputDate_[0] = '\0';
            inputTheme_[0] = '\0';
            inputTemporalLabel_[0] = '\0';
            inputEvidenceSnippet_[0] = '\0';
            inputSourceSection_[0] = '\0';
            inputPageRange_[0] = '\0';
            inputMetadataKey_[0] = '\0';
            inputMetadataValue_[0] = '\0';
            metadata_.clear();
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
    if (history.empty()) {
        selectedObservationIds_.clear();
    } else {
        std::set<std::string> currentIds;
        for (const auto& obs : history) currentIds.insert(obs.id);
        for (auto it = selectedObservationIds_.begin(); it != selectedObservationIds_.end();) {
            if (!currentIds.contains(*it)) it = selectedObservationIds_.erase(it);
            else ++it;
        }
    }

    auto collectForAI = [&]() {
        if (!useSelectedForAI_) return history;
        std::vector<Application::DTO::NarrativeStateDTO> filtered;
        for (const auto& obs : history) {
            if (selectedObservationIds_.contains(obs.id)) filtered.push_back(obs);
        }
        return filtered;
    };

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "OBSERVATION LOG (NOC)");
    ImGui::Checkbox("Use selected observations only", &useSelectedForAI_);
    ImGui::SameLine();
    if (ImGui::Button("Select All##Narrative")) {
        for (const auto& obs : history) selectedObservationIds_.insert(obs.id);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Selection##Narrative")) {
        selectedObservationIds_.clear();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %zu", selectedObservationIds_.size());

    if (aiRequestPending_) {
        ImGui::BeginDisabled();
        ImGui::Button("Waiting for Qwen...", ImVec2(240, 0));
        ImGui::EndDisabled();
    } else if (ImGui::Button("Analyze Themes with Qwen", ImVec2(240, 0))) {
        auto inputHistory = collectForAI();
        if (!inputHistory.empty()) {
            aiRequestPending_ = true;
            auto bundle = Application::Mappers::Cognitive::createBundle("theme_analysis", inputHistory);
            session_->requestAIInterpretation(bundle,
                Application::Services::Cognitive::InterpretationMode::ThemeAnalysis,
                [this](const auto& snapshot) {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    stagedAiSnapshot_ = snapshot;
                    showAiModal_ = true;
                    aiRequestPending_ = false;
                    aiResultReady_ = true; // Signal the main thread to open the popup
                });
        } else {
            if (useSelectedForAI_) ImGui::OpenPopup("NarrativeSelectionEmptyError");
            else ImGui::OpenPopup("NarrativeContextEmptyError");
        }
    }

    if (ImGui::BeginPopupModal("NarrativeSelectionEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Observation Selected");
        ImGui::Text("Select one or more observations before running Qwen.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("NarrativeContextEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Observation Found");
        ImGui::Text("Add at least one observation to run Qwen.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginTable("NarrativeLogTable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Sel");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Intent");
        ImGui::TableSetupColumn("Theme");
        ImGui::TableSetupColumn("Metadata");
        ImGui::TableSetupColumn("Anchor");
        ImGui::TableHeadersRow();

        for (const auto& obs : history) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool selected = selectedObservationIds_.contains(obs.id);
            ImGui::PushID((obs.id + "_sel").c_str());
            if (ImGui::Checkbox("##SelectObservation", &selected)) {
                if (selected) selectedObservationIds_.insert(obs.id);
                else selectedObservationIds_.erase(obs.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(obs.id.c_str());
            if (ImGui::Button("Edit")) {
                loadIntoForm(obs);
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                session_->removeNarrativeStateDTO(obs.id);
                selectedObservationIds_.erase(obs.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", obs.id.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s (%s)", obs.source.sourceId.c_str(), obs.source.productionDate.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", obs.temporalContext.label.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", intentLabel(obs.intent.intentType));

            ImGui::TableSetColumnIndex(6);
            if (!obs.axes.empty()) {
                ImGui::Text("%s", obs.axes[0].label.c_str());
            } else {
                ImGui::Text("-");
            }
            
            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%zu", obs.metadata.size());
            if (ImGui::IsItemHovered() && !obs.metadata.empty()) {
                ImGui::BeginTooltip();
                for (const auto& [key, value] : obs.metadata) {
                    ImGui::TextWrapped("%s: %s", key.c_str(), value.c_str());
                }
                ImGui::EndTooltip();
            }

            ImGui::TableSetColumnIndex(8);
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

void NarrativePanel::drawNarrativeGraph() {
    const auto graph = session_->getNarrativeContextGraph();
    const auto nodeJson = graph.value("nodes", nlohmann::json::array());
    const auto edgeJson = graph.value("edges", nlohmann::json::array());

    std::vector<NarrativeGraphNodeUI> nodes;
    nodes.reserve(nodeJson.size());
    std::unordered_map<std::string, int> nodeIndex;
    for (const auto& n : nodeJson) {
        if (!n.is_object()) continue;

        NarrativeGraphNodeUI node;
        node.id = n.value("id", "");
        node.label = n.value("label", node.id);
        node.dominantDimension = n.value("dominantDimension", "mixed");
        node.dominantIntent = n.value("dominantIntent", "unknown");
        node.narrativeCount = n.value("narrativeCount", 0);

        if (n.contains("topTokens") && n["topTokens"].is_array()) {
            for (const auto& token : n["topTokens"]) {
                if (token.is_string()) node.topTokens.push_back(token.get<std::string>());
            }
        }
        if (n.contains("artifactIds") && n["artifactIds"].is_array()) {
            for (const auto& artifactId : n["artifactIds"]) {
                if (artifactId.is_string()) node.artifactIds.push_back(artifactId.get<std::string>());
            }
        }
        if (n.contains("observationIds") && n["observationIds"].is_array()) {
            for (const auto& observationId : n["observationIds"]) {
                if (observationId.is_string()) node.observationIds.push_back(observationId.get<std::string>());
            }
        }

        node.radius = 11.0f + std::sqrt(static_cast<float>(std::max(1, node.narrativeCount))) * 4.0f;
        nodeIndex[node.id] = static_cast<int>(nodes.size());
        nodes.push_back(std::move(node));
    }

    std::vector<NarrativeGraphEdgeUI> edges;
    edges.reserve(edgeJson.size());
    for (const auto& e : edgeJson) {
        if (!e.is_object()) continue;
        const std::string sourceId = e.value("source", "");
        const std::string targetId = e.value("target", "");
        auto itA = nodeIndex.find(sourceId);
        auto itB = nodeIndex.find(targetId);
        if (itA == nodeIndex.end() || itB == nodeIndex.end()) continue;

        NarrativeGraphEdgeUI edge;
        edge.source = itA->second;
        edge.target = itB->second;
        edge.similarity = e.value("similarity", 0.0f);
        edge.distance = e.value("distance", 1.0f);
        edge.sharedTokens = e.value("sharedTokens", 0);
        edges.push_back(edge);
    }

    auto normalizeDimension = [](std::string dim) {
        if (dim != "ecological" && dim != "productive" && dim != "social") {
            dim = "mixed";
        }
        return dim;
    };

    std::unordered_map<std::string, int> stableNodeLookup;
    stableNodeLookup.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        stableNodeLookup[nodes[i].id] = static_cast<int>(i);
    }
    if (!selectedGraphNodeId_.empty() && !stableNodeLookup.contains(selectedGraphNodeId_)) {
        selectedGraphNodeId_.clear();
    }

    std::vector<bool> edgeActive(edges.size(), false);
    for (size_t i = 0; i < edges.size(); ++i) {
        edgeActive[i] = edges[i].similarity >= graphMinSimilarity_;
    }

    const int topK = std::max(0, graphTopKPerNode_);
    if (topK > 0) {
        std::vector<std::vector<std::pair<float, int>>> incident(nodes.size());
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            incident[edges[i].source].push_back({edges[i].similarity, static_cast<int>(i)});
            incident[edges[i].target].push_back({edges[i].similarity, static_cast<int>(i)});
        }

        std::vector<bool> keep(edges.size(), false);
        for (auto& list : incident) {
            std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
                if (a.first == b.first) return a.second < b.second;
                return a.first > b.first;
            });
            const int limit = std::min<int>(topK, static_cast<int>(list.size()));
            for (int i = 0; i < limit; ++i) {
                keep[list[i].second] = true;
            }
        }

        for (size_t i = 0; i < edges.size(); ++i) {
            edgeActive[i] = edgeActive[i] && keep[i];
        }
    }

    int selectedIndex = -1;
    if (!selectedGraphNodeId_.empty()) {
        auto it = stableNodeLookup.find(selectedGraphNodeId_);
        if (it != stableNodeLookup.end()) selectedIndex = it->second;
    }

    std::set<int> focusNodes;
    if (graphFocusSelected_ && selectedIndex >= 0) {
        focusNodes.insert(selectedIndex);
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            if (edges[i].source == selectedIndex || edges[i].target == selectedIndex) {
                focusNodes.insert(edges[i].source);
                focusNodes.insert(edges[i].target);
            }
        }
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            if (!focusNodes.contains(edges[i].source) || !focusNodes.contains(edges[i].target)) {
                edgeActive[i] = false;
            }
        }
    }

    std::vector<int> degree(nodes.size(), 0);
    size_t activeEdgeCount = 0;
    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edgeActive[i]) continue;
        ++activeEdgeCount;
        degree[edges[i].source] += 1;
        degree[edges[i].target] += 1;
    }

    ImGui::Text("Distance Type: %s", graph.value("distanceType", "unknown").c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| Nodes: %zu | Edges: %zu (%zu shown)", nodes.size(), edges.size(), activeEdgeCount);
    ImGui::SliderFloat("Min Similarity", &graphMinSimilarity_, 0.0f, 1.0f, "%.2f");
    ImGui::SliderInt("Top K links per node", &graphTopKPerNode_, 0, 8);
    ImGui::Checkbox("Show Labels", &graphShowLabels_);
    ImGui::SameLine();
    ImGui::Checkbox("Hide Isolated", &graphHideIsolated_);
    ImGui::SameLine();
    ImGui::Checkbox("Focus Selected Node", &graphFocusSelected_);
    if (graphFocusSelected_ && selectedIndex < 0) {
        ImGui::TextDisabled("Select one node to activate focus mode.");
    }

    if (nodes.empty()) {
        ImGui::TextDisabled("No narrative contexts available to render.");
        return;
    }

    ImGui::BeginChild("NarrativeGraphCanvas", ImVec2(0.0f, 430.0f), true, ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 80.0f || canvasSize.y < 80.0f) {
        ImGui::EndChild();
        return;
    }
    ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);

    ImGui::InvisibleButton("NarrativeGraphButton", canvasSize, ImGuiButtonFlags_MouseButtonLeft);
    const bool canvasHovered = ImGui::IsItemHovered();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(38, 42, 56, 255), 8.0f);
    drawList->AddRect(canvasMin, canvasMax, IM_COL32(95, 110, 130, 180), 8.0f, 0, 1.0f);

    ImVec2 center(canvasMin.x + canvasSize.x * 0.5f, canvasMin.y + canvasSize.y * 0.5f);
    const float maxRing = std::max(50.0f, std::min(canvasSize.x, canvasSize.y) * 0.42f);
    drawList->AddCircle(center, maxRing, IM_COL32(70, 80, 98, 150), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.65f, IM_COL32(70, 80, 98, 130), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.35f, IM_COL32(70, 80, 98, 110), 120, 1.0f);

    std::vector<int> visibleIndices;
    visibleIndices.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (graphFocusSelected_ && selectedIndex >= 0 && !focusNodes.contains(static_cast<int>(i))) continue;
        if (graphHideIsolated_ && degree[i] == 0) continue;
        visibleIndices.push_back(static_cast<int>(i));
    }

    std::vector<bool> nodeVisible(nodes.size(), false);
    for (int idx : visibleIndices) {
        nodeVisible[idx] = true;
        nodes[idx].dominantDimension = normalizeDimension(nodes[idx].dominantDimension);
    }

    std::map<std::string, std::vector<int>> buckets;
    for (int idx : visibleIndices) {
        buckets[nodes[idx].dominantDimension].push_back(idx);
    }
    for (auto& [_, bucket] : buckets) {
        std::sort(bucket.begin(), bucket.end(), [&](int a, int b) {
            if (nodes[a].narrativeCount == nodes[b].narrativeCount) return nodes[a].label < nodes[b].label;
            return nodes[a].narrativeCount > nodes[b].narrativeCount;
        });
    }

    const std::map<std::string, float> sectorCenters = {
        {"ecological", -2.20f},
        {"productive", -0.50f},
        {"social", 1.20f},
        {"mixed", 2.50f}
    };
    const float sectorSpan = 1.10f;
    for (const auto& [dim, centerAngle] : sectorCenters) {
        auto it = buckets.find(dim);
        if (it == buckets.end()) continue;
        const auto& bucket = it->second;
        const size_t n = bucket.size();
        for (size_t i = 0; i < n; ++i) {
            const float t = (n <= 1) ? 0.5f : (static_cast<float>(i) / static_cast<float>(n - 1));
            const float angle = centerAngle - sectorSpan * 0.5f + sectorSpan * t;
            const float ring = static_cast<float>(i % 3);
            const float layer = static_cast<float>(i / 3);
            const float radius = maxRing * (0.42f + 0.16f * ring) + 10.0f * layer;
            NarrativeGraphNodeUI& node = nodes[bucket[i]];
            node.position = ImVec2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        }
    }

    int hoveredNode = -1;
    if (canvasHovered) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        for (int idx : visibleIndices) {
            const auto& node = nodes[idx];
            const float dx = mouse.x - node.position.x;
            const float dy = mouse.y - node.position.y;
            if ((dx * dx + dy * dy) <= (node.radius + 4.0f) * (node.radius + 4.0f)) {
                hoveredNode = idx;
                break;
            }
        }
    }

    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edgeActive[i]) continue;
        const auto& edge = edges[i];
        if (!nodeVisible[edge.source] || !nodeVisible[edge.target]) continue;

        const ImVec2 a = nodes[edge.source].position;
        const ImVec2 b = nodes[edge.target].position;
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float length = std::sqrt(dx * dx + dy * dy);
        if (length < 1.0f) continue;

        const float nx = -dy / length;
        const float ny = dx / length;
        const float curvature = std::min(40.0f, length * 0.12f);
        const ImVec2 c1(a.x + dx * 0.33f + nx * curvature, a.y + dy * 0.33f + ny * curvature);
        const ImVec2 c2(a.x + dx * 0.66f + nx * curvature, a.y + dy * 0.66f + ny * curvature);

        const bool selectedEdge = selectedIndex >= 0 && (edge.source == selectedIndex || edge.target == selectedIndex);
        const int alpha = static_cast<int>((selectedEdge ? 120 : 55) + edge.similarity * 120.0f);
        const float thickness = (selectedEdge ? 1.8f : 0.8f) + edge.similarity * 2.4f;
        const ImU32 edgeColor = selectedEdge ? IM_COL32(255, 222, 120, alpha) : IM_COL32(130, 162, 220, alpha);
        drawList->AddBezierCubic(a, c1, c2, b, edgeColor, thickness);
    }

    for (int idx : visibleIndices) {
        auto& node = nodes[idx];
        const bool selected = (selectedGraphNodeId_ == node.id);
        const bool hovered = (hoveredNode == idx);

        const ImU32 baseColor = colorForDimension(node.dominantDimension);
        const float glowRadius = node.radius + (selected ? 7.0f : 4.0f);
        drawList->AddCircleFilled(node.position, glowRadius, IM_COL32(80, 110, 180, selected ? 70 : 30), 36);
        drawList->AddCircleFilled(node.position, node.radius, baseColor, 36);
        drawList->AddCircle(node.position, node.radius, selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(210, 220, 240, hovered ? 255 : 160), 36, selected ? 2.5f : 1.4f);

        if (graphShowLabels_ || hovered || selected) {
            const std::string label = node.label.size() > 36 ? node.label.substr(0, 33) + "..." : node.label;
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f + 1.0f, node.position.y - node.radius - textSize.y - 7.0f + 1.0f), IM_COL32(0, 0, 0, 185), label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f, node.position.y - node.radius - textSize.y - 7.0f), IM_COL32(230, 236, 245, 245), label.c_str());
        }

        const std::string countLabel = std::to_string(node.narrativeCount);
        ImVec2 countSize = ImGui::CalcTextSize(countLabel.c_str());
        drawList->AddText(ImVec2(node.position.x - countSize.x * 0.5f, node.position.y - countSize.y * 0.5f), IM_COL32(20, 24, 35, 245), countLabel.c_str());
    }

    if (canvasHovered && hoveredNode >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        selectedGraphNodeId_ = nodes[hoveredNode].id;
    }
    if (canvasHovered && hoveredNode >= 0) {
        const auto& node = nodes[hoveredNode];
        ImGui::BeginTooltip();
        ImGui::Text("Context: %s", node.label.c_str());
        ImGui::Text("Narratives: %d", node.narrativeCount);
        ImGui::Text("Dimension: %s", labelForDimension(node.dominantDimension));
        ImGui::Text("Intent: %s", node.dominantIntent.c_str());
        if (!node.observationIds.empty()) {
            ImGui::Text("Observation IDs: %zu", node.observationIds.size());
        }
        if (!node.artifactIds.empty()) {
            ImGui::Text("Artifacts: %zu", node.artifactIds.size());
        }
        if (!node.topTokens.empty()) {
            std::string top;
            for (size_t i = 0; i < node.topTokens.size(); ++i) {
                if (i > 0) top += ", ";
                top += node.topTokens[i];
                if (top.size() > 170) {
                    top += "...";
                    break;
                }
            }
            ImGui::TextWrapped("Top Tokens: %s", top.c_str());
        }
        ImGui::EndTooltip();
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Legend");
    drawDimensionLegendItem("##LegendEcological", colorForDimension("ecological"), "Ecological");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendProductive", colorForDimension("productive"), "Productive");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendSocial", colorForDimension("social"), "Social");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendMixed", colorForDimension("mixed"), "Mixed");
    ImGui::BulletText("Node color: dominant epistemic dimension");
    ImGui::BulletText("Node size: number of narrative observations");
    ImGui::BulletText("Edge width/alpha: narrative similarity (Jaccard)");
    ImGui::BulletText("Edges filtered by Min Similarity and Top-K per node");
    ImGui::BulletText("Focus mode keeps only the selected node and first-hop neighbors");

    if (!selectedGraphNodeId_.empty()) {
        auto itSelected = std::find_if(nodes.begin(), nodes.end(), [&](const auto& item) {
            return item.id == selectedGraphNodeId_;
        });
        if (itSelected != nodes.end()) {
            ImGui::Separator();
            ImGui::Text("Selected Context: %s", itSelected->label.c_str());
            ImGui::TextDisabled("Dominant Dimension: %s", labelForDimension(itSelected->dominantDimension));
            ImGui::TextDisabled("Dominant Intent: %s", itSelected->dominantIntent.c_str());
            ImGui::TextDisabled("Narrative Count: %d", itSelected->narrativeCount);

            if (!itSelected->observationIds.empty()) {
                ImGui::Text("Observation IDs");
                ImGui::BeginChild("NarrativeGraphObservationIds", ImVec2(0.0f, 74.0f), true);
                for (const auto& id : itSelected->observationIds) {
                    ImGui::BulletText("%s", id.c_str());
                }
                ImGui::EndChild();
            }

            if (!itSelected->artifactIds.empty()) {
                ImGui::Text("IW Artifacts");
                ImGui::BeginChild("NarrativeGraphArtifactIds", ImVec2(0.0f, 64.0f), true);
                for (const auto& artifactId : itSelected->artifactIds) {
                    ImGui::BulletText("%s", artifactId.c_str());
                }
                ImGui::EndChild();
            }
        }
    }
}

void NarrativePanel::loadIntoForm(const Application::DTO::NarrativeStateDTO& dto) {
    isEditing_ = true;
    editingId_ = dto.id;
    
    // Source
    strncpy(inputSourceId_, dto.source.sourceId.c_str(), sizeof(inputSourceId_) - 1);
    inputSourceId_[sizeof(inputSourceId_) - 1] = '\0';
    strncpy(inputDate_, dto.source.productionDate.c_str(), sizeof(inputDate_) - 1);
    inputDate_[sizeof(inputDate_) - 1] = '\0';
    
    for (int i = 0; i < IM_ARRAYSIZE(SOURCE_TYPE_VALUES); i++) {
        if (dto.source.sourceType == SOURCE_TYPE_VALUES[i]) {
            inputSourceType_ = i;
            break;
        }
    }

    // Time
    strncpy(inputTemporalLabel_, dto.temporalContext.label.c_str(), sizeof(inputTemporalLabel_) - 1);
    inputTemporalLabel_[sizeof(inputTemporalLabel_) - 1] = '\0';
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
        inputTheme_[sizeof(inputTheme_) - 1] = '\0';
    } else {
        inputTheme_[0] = '\0';
    }

    metadata_ = dto.metadata;
    const std::string evidenceSnippet = metadataValueByPriority(metadata_, {"iw.evidenceSnippet", "evidenceSnippet"});
    const std::string sourceSection = metadataValueByPriority(metadata_, {"iw.sourceSection", "sourceSection"});
    const std::string pageRange = metadataValueByPriority(metadata_, {"iw.pageRange", "pageRange"});
    strncpy(inputEvidenceSnippet_, evidenceSnippet.c_str(), sizeof(inputEvidenceSnippet_) - 1);
    inputEvidenceSnippet_[sizeof(inputEvidenceSnippet_) - 1] = '\0';
    strncpy(inputSourceSection_, sourceSection.c_str(), sizeof(inputSourceSection_) - 1);
    inputSourceSection_[sizeof(inputSourceSection_) - 1] = '\0';
    strncpy(inputPageRange_, pageRange.c_str(), sizeof(inputPageRange_) - 1);
    inputPageRange_[sizeof(inputPageRange_) - 1] = '\0';
    inputMetadataKey_[0] = '\0';
    inputMetadataValue_[0] = '\0';

    // Spatial
    capturedScope_ = dto.spatialScope;
}

} // namespace UI::Panels
