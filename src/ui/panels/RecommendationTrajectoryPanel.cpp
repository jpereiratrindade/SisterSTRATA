#include "src/ui/panels/RecommendationTrajectoryPanel.hpp"
#include "imgui.h"
#include <cstring>
#include <filesystem>
#include "application/mappers/CognitiveMappers.hpp"
#include "ui/components/InterpretationModal.hpp"
#include "ui/components/InterpretationHistory.hpp"
#include <algorithm>
#include <vector>

static const char* REC_SOURCE_LABELS[] = {
    "Technical Recommendation", "Technical Bulletin", "Report", "Document", "Other"
};

static const char* REC_SOURCE_VALUES[] = {
    "TECHNICAL_RECOMMENDATION", "TECHNICAL_BULLETIN", "REPORT", "DOCUMENT", "OTHER"
};

static const char* TEMPORAL_LABELS[] = {
    "Ancestral", "Past", "Recent Past", "Contemporary", "Future Vision", "Timeless", "Indeterminate"
};

static const char* TEMPORAL_VALUES[] = {
    "ANCESTRAL", "PAST", "RECENT_PAST", "CONTEMPORARY", "FUTURE_VISION", "TIMELESS", "INDETERMINATE"
};

namespace UI::Panels {

void RecommendationTrajectoryPanel::setSession(Application::Session* session) {
    session_ = session;
}

void RecommendationTrajectoryPanel::draw(bool* open) {
    if (!open || !*open) return;

    ImGui::SetNextWindowSize(ImVec2(880, 660), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Recommendation Trajectory Context", open)) {
        if (!session_) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: No Session Connected");
            ImGui::End();
            return;
        }

        if (ImGui::BeginTabBar("RecommendationTabs")) {
            
            if (ImGui::BeginTabItem("Trajectory", nullptr, targetTab_ == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {
                // Check for Deferred AI Results (Thread-safe UI update)
                {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    if (aiResultReady_) {
                        lastAiSnapshot_ = stagedAiSnapshot_; // Safe copy
                        ImGui::OpenPopup("AI Recommendation Analysis");
                        aiResultReady_ = false;
                    }
                }
                
                drawTrajectoryConfig();
                
                ImGui::Separator();
                drawSnapshotList();
                
                // AI Modal Rendering
                UI::Components::InterpretationModal::Draw("AI Recommendation Analysis", showAiModal_, lastAiSnapshot_, [this](const auto& snap) {
                    session_->saveInterpretationSnapshotDTO(snap);
                });

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Snapshots", nullptr, targetTab_ == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {
                drawSnapshotForm();
                ImGui::Separator();
                drawSnapshotList();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Epistemic Memory", nullptr, targetTab_ == 2 ? ImGuiTabItemFlags_SetSelected : 0)) {
                auto snapshots = session_->getInterpretationSnapshots();
                std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> filtered;

                // Filter for Recommendation Context
                std::copy_if(snapshots.begin(), snapshots.end(), std::back_inserter(filtered), [](const auto& s){
                    return s.intent == "trajectory_reading";
                });

                std::reverse(filtered.begin(), filtered.end());
                UI::Components::InterpretationHistory::Draw(filtered);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
            
            targetTab_ = -1; // Reset after one frame of "Selection"
        }
    }
    ImGui::End();
}

void RecommendationTrajectoryPanel::drawTrajectoryConfig() {
    auto current = session_->getRecommendationTrajectoryDTO();
    if (current.snapshots.empty()) {
        selectedSnapshotIds_.clear();
    } else {
        std::set<std::string> currentIds;
        for (const auto& snapshot : current.snapshots) currentIds.insert(snapshot.id);
        for (auto it = selectedSnapshotIds_.begin(); it != selectedSnapshotIds_.end();) {
            if (!currentIds.contains(*it)) it = selectedSnapshotIds_.erase(it);
            else ++it;
        }
    }

    auto collectTrajectoryForAI = [&]() {
        auto selected = current;
        if (!useSelectedForAI_) {
            return selected;
        }
        selected.snapshots.clear();
        for (const auto& snapshot : current.snapshots) {
            if (selectedSnapshotIds_.contains(snapshot.id)) {
                selected.snapshots.push_back(snapshot);
            }
        }
        return selected;
    };

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "RECOMENDATION TRAJECTORY (RTC)");
    ImGui::SameLine(ImGui::GetWindowWidth() - 280);

    if (aiRequestPending_) {
        ImGui::BeginDisabled();
        ImGui::Button("Waiting for Qwen...", ImVec2(240, 0));
        ImGui::EndDisabled();
    } else if (ImGui::Button("Analyze Trajectory with Qwen", ImVec2(240, 0))) {
        auto trajectory = collectTrajectoryForAI();
        if (!trajectory.snapshots.empty()) {
            aiRequestPending_ = true;
            auto bundle = Application::Mappers::Cognitive::createBundle("trajectory_reading", {}, {}, &trajectory);
            session_->requestAIInterpretation(bundle,
                Application::Services::Cognitive::InterpretationMode::TrajectoryReading,
                [this](const auto& snapshot) {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    stagedAiSnapshot_ = snapshot;
                    showAiModal_ = true;
                    aiRequestPending_ = false;
                    aiResultReady_ = true; // Signal the main thread to open the popup
                });
        } else {
            if (useSelectedForAI_) ImGui::OpenPopup("RecommendationSelectionEmptyError");
            else ImGui::OpenPopup("SnapshotsEmptyError");
        }
    }

    ImGui::Spacing();
    ImGui::Checkbox("Use selected snapshots only", &useSelectedForAI_);
    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %zu", selectedSnapshotIds_.size());
    
    if (ImGui::BeginPopupModal("SnapshotsEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Snapshots Found");
        ImGui::Text("The AI needs at least one Recommendation Snapshot to analyze a trajectory.");
        ImGui::Text("Please add a snapshot in the 'Snapshots' tab first.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("RecommendationSelectionEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Snapshot Selected");
        ImGui::Text("Select one or more snapshots in the table before running Qwen.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();

    ImGui::Text("Current Trajectory ID: %s", current.id.empty() ? "(not set)" : current.id.c_str());
    ImGui::InputText("Trajectory ID", inputTrajectoryId_, IM_ARRAYSIZE(inputTrajectoryId_));

    ImGui::Spacing();
    ImGui::Text("Metadata");
    ImGui::InputText("Key", inputMetadataKey_, IM_ARRAYSIZE(inputMetadataKey_));
    ImGui::InputText("Value", inputMetadataValue_, IM_ARRAYSIZE(inputMetadataValue_));
    if (ImGui::Button("Add Metadata")) {
        if (strlen(inputMetadataKey_) > 0) {
            pendingMetadata_[inputMetadataKey_] = inputMetadataValue_;
            inputMetadataKey_[0] = '\0';
            inputMetadataValue_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Pending")) {
        pendingMetadata_.clear();
    }

    if (!pendingMetadata_.empty()) {
        ImGui::TextDisabled("Pending Metadata:");
        for (const auto& [key, value] : pendingMetadata_) {
            ImGui::BulletText("%s: %s", key.c_str(), value.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Apply Trajectory", ImVec2(200, 0))) {
        Application::DTO::RecommendationTrajectoryDTO dto;
        dto.id = (strlen(inputTrajectoryId_) > 0) ? std::string(inputTrajectoryId_)
                                                  : (current.id.empty() ? "REC-TRAJECTORY-1" : current.id);

        dto.snapshots = current.snapshots;
        dto.metadata = current.metadata;
        for (const auto& [key, value] : pendingMetadata_) {
            dto.metadata[key] = value;
        }

        session_->setRecommendationTrajectoryDTO(dto);
        pendingMetadata_.clear();
        inputTrajectoryId_[0] = '\0';
        ImGui::OpenPopup("TrajectoryApplied");
    }

    if (ImGui::BeginPopupModal("TrajectoryApplied", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Trajectory updated successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (!current.metadata.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("Current Metadata:");
        for (const auto& [key, value] : current.metadata) {
            ImGui::BulletText("%s: %s", key.c_str(), value.c_str());
        }
    }
}

void RecommendationTrajectoryPanel::drawSnapshotForm() {
    ImGui::TextDisabled("Register Recommendation Snapshot");
    ImGui::Separator();

    ImGui::InputText("Snapshot ID (optional)", inputSnapshotId_, IM_ARRAYSIZE(inputSnapshotId_));
    ImGui::InputTextMultiline("Recommendation Text", inputRecommendationText_, IM_ARRAYSIZE(inputRecommendationText_), ImVec2(-1, 80));
    ImGui::InputText("Intended Action", inputIntendedAction_, IM_ARRAYSIZE(inputIntendedAction_));
    ImGui::InputText("Expected Outcome", inputExpectedOutcome_, IM_ARRAYSIZE(inputExpectedOutcome_));

    ImGui::Spacing();
    ImGui::Text("Context Conditions");
    ImGui::InputText("Condition", inputContextCondition_, IM_ARRAYSIZE(inputContextCondition_));
    ImGui::SameLine();
    if (ImGui::Button("Add Condition")) {
        if (strlen(inputContextCondition_) > 0) {
            contextConditions_.push_back(inputContextCondition_);
            inputContextCondition_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Conditions")) {
        contextConditions_.clear();
    }

    if (!contextConditions_.empty()) {
        ImGui::TextDisabled("Conditions:");
        for (const auto& item : contextConditions_) {
            ImGui::BulletText("%s", item.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Text("Source Reference");
    ImGui::InputText("Source ID", inputSourceId_, IM_ARRAYSIZE(inputSourceId_));
    ImGui::InputText("Date/Year", inputSourceDate_, IM_ARRAYSIZE(inputSourceDate_));
    ImGui::InputText("Author (optional)", inputSourceAuthor_, IM_ARRAYSIZE(inputSourceAuthor_));
    ImGui::Combo("Type", &inputSourceType_, REC_SOURCE_LABELS, IM_ARRAYSIZE(REC_SOURCE_LABELS));

    ImGui::Spacing();
    ImGui::Text("Temporal Context (Declared)");
    ImGui::Combo("Category", &inputTemporalCategory_, TEMPORAL_LABELS, IM_ARRAYSIZE(TEMPORAL_LABELS));
    ImGui::InputText("Label", inputTemporalLabel_, IM_ARRAYSIZE(inputTemporalLabel_));

    ImGui::Spacing();
    ImGui::Separator();

    if (isEditing_) {
        ImGui::TextColored(ImVec4(1,1,0,1), "EDITING MODE: %s", editingId_.c_str());
        if (ImGui::Button("Cancel Edit")) {
            isEditing_ = false;
            editingId_.clear();
            // Clear inputs
            contextConditions_.clear();
            inputSnapshotId_[0] = '\0';
            inputRecommendationText_[0] = '\0';
            inputIntendedAction_[0] = '\0';
            inputExpectedOutcome_[0] = '\0';
        }
        ImGui::SameLine();
    }

    if (ImGui::Button(isEditing_ ? "Save Changes" : "Add Snapshot", ImVec2(200, 0))) {
        Application::DTO::RecommendationSnapshotDTO dto;
        if (strlen(inputSnapshotId_) > 0) {
            dto.id = inputSnapshotId_;
        } else {
            dto.id = "REC-" + std::to_string(session_->getRecommendationSnapshotCount() + 1);
        }
        dto.recommendationText = inputRecommendationText_;
        dto.contextConditions = contextConditions_; // Auto-add? Ideally yes, but sticking to button for now or user request.
        // User pattern from DISC: Check buffers.
        if (strlen(inputContextCondition_) > 0) dto.contextConditions.push_back(inputContextCondition_);
        
        dto.intendedAction = inputIntendedAction_;
        dto.expectedOutcome = inputExpectedOutcome_;
        dto.sourceReference.sourceType = REC_SOURCE_VALUES[inputSourceType_];
        dto.sourceReference.sourceId = inputSourceId_;
        dto.sourceReference.productionDate = inputSourceDate_;
        if (strlen(inputSourceAuthor_) > 0) {
            dto.sourceReference.author = std::string(inputSourceAuthor_);
        }
        dto.temporalContext = Application::DTO::TemporalContextDTO{
            TEMPORAL_VALUES[inputTemporalCategory_],
            inputTemporalLabel_
        };

        try {
            if (isEditing_) {
                session_->updateRecommendationSnapshotDTO(editingId_, dto);
                ImGui::OpenPopup("SnapshotUpdateSuccess");
                isEditing_ = false;
                editingId_.clear();
            } else {
                session_->addRecommendationSnapshotDTO(dto);
                ImGui::OpenPopup("SnapshotSuccess");
            }
            contextConditions_.clear();
            inputSnapshotId_[0] = '\0';
            inputRecommendationText_[0] = '\0';
            inputIntendedAction_[0] = '\0';
            inputExpectedOutcome_[0] = '\0';
            inputContextCondition_[0] = '\0'; // Clear buffer
        } catch (const std::exception&) {
            ImGui::OpenPopup("SnapshotError");
        }
    }

    if (ImGui::BeginPopupModal("SnapshotSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Snapshot registered successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("SnapshotUpdateSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Snapshot updated successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("SnapshotError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to register snapshot. Check IDs or duplicates.");
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
    if (importSelector_.draw(&showImportDialog_, importResult, ".recommendation.json")) {
        try {
            session_->loadRecommendationTrajectoryFromFile(importResult);
            std::filesystem::path selected(importResult);
            if (selected.has_parent_path()) {
                lastImportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("RecommendationImportSuccess");
        } catch (const std::exception&) {
            ImGui::OpenPopup("RecommendationImportError");
        }
        showImportDialog_ = false;
    }

    std::string exportResult;
    if (exportSelector_.draw(&showExportDialog_, exportResult, ".recommendation.json", true)) {
        try {
            session_->saveRecommendationTrajectoryToFile(exportResult);
            std::filesystem::path selected(exportResult);
            if (selected.has_parent_path()) {
                lastExportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("RecommendationExportSuccess");
        } catch (const std::exception&) {
            ImGui::OpenPopup("RecommendationExportError");
        }
        showExportDialog_ = false;
    }

    if (ImGui::BeginPopupModal("RecommendationImportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Recommendation trajectory imported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("RecommendationImportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to import recommendation trajectory.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("RecommendationExportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Recommendation trajectory exported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("RecommendationExportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to export recommendation trajectory.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void RecommendationTrajectoryPanel::drawSnapshotList() {
    auto trajectory = session_->getRecommendationTrajectoryDTO();
    if (trajectory.snapshots.empty()) {
        selectedSnapshotIds_.clear();
        ImGui::TextDisabled("No recommendation snapshots registered.");
        return;
    }

    std::set<std::string> currentIds;
    for (const auto& snapshot : trajectory.snapshots) currentIds.insert(snapshot.id);
    for (auto it = selectedSnapshotIds_.begin(); it != selectedSnapshotIds_.end();) {
        if (!currentIds.contains(*it)) it = selectedSnapshotIds_.erase(it);
        else ++it;
    }

    if (ImGui::Button("Select All##Recommendation")) {
        selectedSnapshotIds_ = currentIds;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Selection##Recommendation")) {
        selectedSnapshotIds_.clear();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Selected: %zu", selectedSnapshotIds_.size());

    if (ImGui::BeginTable("RecommendationSnapshotsTable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Sel");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Action");
        ImGui::TableSetupColumn("Outcome");
        ImGui::TableSetupColumn("Conditions");
        ImGui::TableHeadersRow();

        for (const auto& snapshot : trajectory.snapshots) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool selected = selectedSnapshotIds_.contains(snapshot.id);
            ImGui::PushID((snapshot.id + "_sel").c_str());
            if (ImGui::Checkbox("##SelectRecommendationSnapshot", &selected)) {
                if (selected) selectedSnapshotIds_.insert(snapshot.id);
                else selectedSnapshotIds_.erase(snapshot.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(snapshot.id.c_str());
            if (ImGui::Button("Edit")) {
                loadIntoForm(snapshot);
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                session_->removeRecommendationSnapshotDTO(snapshot.id);
                selectedSnapshotIds_.erase(snapshot.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", snapshot.id.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", snapshot.sourceReference.sourceId.c_str());

            ImGui::TableSetColumnIndex(4);
            std::string timeLabel = snapshot.temporalContext.label;
            const bool isGenericIWTime =
                (timeLabel == "IW recommendation ingestion") || (timeLabel == "IW ingestion");
            if ((timeLabel.empty() || isGenericIWTime) && !snapshot.sourceReference.productionDate.empty()) {
                timeLabel = snapshot.sourceReference.productionDate;
            }
            if (timeLabel.empty()) {
                timeLabel = "-";
            }
            ImGui::Text("%s", timeLabel.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", snapshot.intendedAction.c_str());

            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s", snapshot.expectedOutcome.c_str());

            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%zu", snapshot.contextConditions.size());
        }
        ImGui::EndTable();
    }
}

void RecommendationTrajectoryPanel::loadIntoForm(const Application::DTO::RecommendationSnapshotDTO& dto) {
    isEditing_ = true;
    editingId_ = dto.id;
    targetTab_ = 1; // Switch to Snapshots tab for editing

    strncpy(inputSnapshotId_, dto.id.c_str(), sizeof(inputSnapshotId_) - 1);
    strncpy(inputRecommendationText_, dto.recommendationText.c_str(), sizeof(inputRecommendationText_) - 1);
    strncpy(inputIntendedAction_, dto.intendedAction.c_str(), sizeof(inputIntendedAction_) - 1);
    strncpy(inputExpectedOutcome_, dto.expectedOutcome.c_str(), sizeof(inputExpectedOutcome_) - 1);
    
    contextConditions_ = dto.contextConditions;
    
    // Source
    strncpy(inputSourceId_, dto.sourceReference.sourceId.c_str(), sizeof(inputSourceId_) - 1);
    strncpy(inputSourceDate_, dto.sourceReference.productionDate.c_str(), sizeof(inputSourceDate_) - 1);
     // Find source enum... simple loop
    for (int i = 0; i < IM_ARRAYSIZE(REC_SOURCE_VALUES); i++) {
        if (dto.sourceReference.sourceType == REC_SOURCE_VALUES[i]) {
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
}

} // namespace UI::Panels
