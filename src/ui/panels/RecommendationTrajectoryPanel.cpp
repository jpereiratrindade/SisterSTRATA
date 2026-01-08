#include "src/ui/panels/RecommendationTrajectoryPanel.hpp"
#include "imgui.h"
#include <cstring>
#include <filesystem>

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
            if (ImGui::BeginTabItem("Trajectory")) {
                drawTrajectoryConfig();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Snapshots")) {
                drawSnapshotForm();
                ImGui::Separator();
                drawSnapshotList();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void RecommendationTrajectoryPanel::drawTrajectoryConfig() {
    auto current = session_->getRecommendationTrajectoryDTO();

    ImGui::TextDisabled("Recommendation Trajectory Setup");
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
        ImGui::TextDisabled("No recommendation snapshots registered.");
        return;
    }

    if (ImGui::BeginTable("RecommendationSnapshotsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
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
            ImGui::PushID(snapshot.id.c_str());
            if (ImGui::Button("Edit")) {
                loadIntoForm(snapshot);
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                session_->removeRecommendationSnapshotDTO(snapshot.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", snapshot.id.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", snapshot.sourceReference.sourceId.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", snapshot.temporalContext.label.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", snapshot.intendedAction.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", snapshot.expectedOutcome.c_str());

            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%zu", snapshot.contextConditions.size());
        }
        ImGui::EndTable();
    }
}

void RecommendationTrajectoryPanel::loadIntoForm(const Application::DTO::RecommendationSnapshotDTO& dto) {
    isEditing_ = true;
    editingId_ = dto.id;

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
