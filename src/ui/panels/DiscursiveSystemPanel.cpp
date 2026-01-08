#include "src/ui/panels/DiscursiveSystemPanel.hpp"
#include "imgui.h"
#include <cstring>
#include <filesystem>

static const char* DISC_SOURCE_LABELS[] = {
    "Interview", "Document", "Questionnaire", "Technical Bulletin", "Report", "Other"
};

static const char* DISC_SOURCE_VALUES[] = {
    "INTERVIEW", "DOCUMENT", "QUESTIONNAIRE", "TECHNICAL_BULLETIN", "REPORT", "OTHER"
};

static const char* TEMPORAL_LABELS[] = {
    "Ancestral", "Past", "Recent Past", "Contemporary", "Future Vision", "Timeless", "Indeterminate"
};

static const char* TEMPORAL_VALUES[] = {
    "ANCESTRAL", "PAST", "RECENT_PAST", "CONTEMPORARY", "FUTURE_VISION", "TIMELESS", "INDETERMINATE"
};

namespace UI::Panels {

void DiscursiveSystemPanel::setSession(Application::Session* session) {
    session_ = session;
}

void DiscursiveSystemPanel::draw(bool* open) {
    if (!open || !*open) return;

    ImGui::SetNextWindowSize(ImVec2(860, 640), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Discursive System Context", open)) {
        if (!session_) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: No Session Connected");
            ImGui::End();
            return;
        }

        if (ImGui::BeginTabBar("DiscursiveTabs")) {
            if (ImGui::BeginTabItem("Ingestion")) {
                drawIngestionForm();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Registered Systems")) {
                drawSystemList();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void DiscursiveSystemPanel::drawIngestionForm() {
    ImGui::TextDisabled("Declarative Ingestion of Discursive Systems");
    ImGui::Separator();

    ImGui::Text("System ID (optional)");
    ImGui::InputText("System ID", inputSystemId_, IM_ARRAYSIZE(inputSystemId_));

    ImGui::Spacing();

    ImGui::Text("Source Reference");
    ImGui::InputText("Source ID", inputSourceId_, IM_ARRAYSIZE(inputSourceId_));
    ImGui::InputText("Date/Year", inputSourceDate_, IM_ARRAYSIZE(inputSourceDate_));
    ImGui::InputText("Author (optional)", inputSourceAuthor_, IM_ARRAYSIZE(inputSourceAuthor_));
    ImGui::Combo("Type", &inputSourceType_, DISC_SOURCE_LABELS, IM_ARRAYSIZE(DISC_SOURCE_LABELS));
    if (ImGui::Button("Add Source")) {
        if (strlen(inputSourceId_) > 0) {
            Application::DTO::SourceReferenceDTO source;
            source.sourceType = DISC_SOURCE_VALUES[inputSourceType_];
            source.sourceId = inputSourceId_;
            source.productionDate = inputSourceDate_;
            if (strlen(inputSourceAuthor_) > 0) {
                source.author = std::string(inputSourceAuthor_);
            }
            sourceReferences_.push_back(source);
            inputSourceId_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Sources")) {
        sourceReferences_.clear();
    }

    if (!sourceReferences_.empty()) {
        ImGui::TextDisabled("Sources:");
        for (const auto& src : sourceReferences_) {
            ImGui::BulletText("%s (%s)", src.sourceId.c_str(), src.productionDate.c_str());
        }
    }

    ImGui::Spacing();

    ImGui::Text("Temporal Context (Declared)");
    ImGui::Combo("Category", &inputTemporalCategory_, TEMPORAL_LABELS, IM_ARRAYSIZE(TEMPORAL_LABELS));
    ImGui::InputText("Label", inputTemporalLabel_, IM_ARRAYSIZE(inputTemporalLabel_));

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Declared Problems");
    ImGui::InputText("Problem", inputProblem_, IM_ARRAYSIZE(inputProblem_));
    ImGui::SameLine();
    if (ImGui::Button("Add Problem")) {
        if (strlen(inputProblem_) > 0) {
            declaredProblems_.push_back(inputProblem_);
            inputProblem_[0] = '\0';
        }
    }

    ImGui::Text("Declared Actions");
    ImGui::InputText("Action", inputAction_, IM_ARRAYSIZE(inputAction_));
    ImGui::SameLine();
    if (ImGui::Button("Add Action")) {
        if (strlen(inputAction_) > 0) {
            declaredActions_.push_back(inputAction_);
            inputAction_[0] = '\0';
        }
    }

    ImGui::Text("Alleged Mechanisms");
    ImGui::InputText("Mechanism", inputMechanism_, IM_ARRAYSIZE(inputMechanism_));
    ImGui::SameLine();
    if (ImGui::Button("Add Mechanism")) {
        if (strlen(inputMechanism_) > 0) {
            allegedMechanisms_.push_back(inputMechanism_);
            inputMechanism_[0] = '\0';
        }
    }

    ImGui::Text("Expected Effects");
    ImGui::InputText("Effect", inputEffect_, IM_ARRAYSIZE(inputEffect_));
    ImGui::SameLine();
    if (ImGui::Button("Add Effect")) {
        if (strlen(inputEffect_) > 0) {
            expectedEffects_.push_back(inputEffect_);
            inputEffect_[0] = '\0';
        }
    }

    ImGui::Spacing();

    if (!declaredProblems_.empty() || !declaredActions_.empty() || !allegedMechanisms_.empty() || !expectedEffects_.empty()) {
        ImGui::TextDisabled("Current Items:");
        for (const auto& item : declaredProblems_) ImGui::BulletText("Problem: %s", item.c_str());
        for (const auto& item : declaredActions_) ImGui::BulletText("Action: %s", item.c_str());
        for (const auto& item : allegedMechanisms_) ImGui::BulletText("Mechanism: %s", item.c_str());
        for (const auto& item : expectedEffects_) ImGui::BulletText("Effect: %s", item.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Interpretation Metadata");
    ImGui::InputText("Key", inputMetadataKey_, IM_ARRAYSIZE(inputMetadataKey_));
    ImGui::InputText("Value", inputMetadataValue_, IM_ARRAYSIZE(inputMetadataValue_));
    if (ImGui::Button("Add Metadata")) {
        if (strlen(inputMetadataKey_) > 0) {
            metadata_[inputMetadataKey_] = inputMetadataValue_;
            inputMetadataKey_[0] = '\0';
            inputMetadataValue_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Metadata")) {
        metadata_.clear();
    }

    if (!metadata_.empty()) {
        ImGui::TextDisabled("Metadata:");
        for (const auto& [key, value] : metadata_) {
            ImGui::BulletText("%s: %s", key.c_str(), value.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Register Discursive System", ImVec2(240, 0))) {
        Application::DTO::DiscursiveSystemDTO dto;

        if (strlen(inputSystemId_) > 0) {
            dto.id = inputSystemId_;
        } else {
            dto.id = "DS-" + std::to_string(session_->getDiscursiveSystemCount() + 1);
        }

        dto.declaredProblems = declaredProblems_;
        dto.declaredActions = declaredActions_;
        dto.allegedMechanisms = allegedMechanisms_;
        dto.expectedEffects = expectedEffects_;
        dto.temporalContext = Application::DTO::TemporalContextDTO{
            TEMPORAL_VALUES[inputTemporalCategory_],
            inputTemporalLabel_
        };
        dto.interpretationMetadata = metadata_;

        dto.sourceReferences = sourceReferences_;
        if (dto.sourceReferences.empty() && strlen(inputSourceId_) > 0) {
            Application::DTO::SourceReferenceDTO source;
            source.sourceType = DISC_SOURCE_VALUES[inputSourceType_];
            source.sourceId = inputSourceId_;
            source.productionDate = inputSourceDate_;
            if (strlen(inputSourceAuthor_) > 0) {
                source.author = std::string(inputSourceAuthor_);
            }
            dto.sourceReferences.push_back(source);
        }

        try {
            session_->registerDiscursiveSystemDTO(dto);
            ImGui::OpenPopup("DiscursiveSuccess");
            declaredProblems_.clear();
            declaredActions_.clear();
            allegedMechanisms_.clear();
            expectedEffects_.clear();
            metadata_.clear();
            inputSystemId_[0] = '\0';
        } catch (const std::exception&) {
            ImGui::OpenPopup("DiscursiveError");
        }
    }

    if (ImGui::BeginPopupModal("DiscursiveSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Discursive system registered successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("DiscursiveError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to register discursive system. Check IDs or duplicates.");
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
    if (importSelector_.draw(&showImportDialog_, importResult, ".discursive.json")) {
        try {
            session_->loadDiscursiveSystemsFromFile(importResult);
            std::filesystem::path selected(importResult);
            if (selected.has_parent_path()) {
                lastImportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("DiscursiveImportSuccess");
        } catch (const std::exception&) {
            ImGui::OpenPopup("DiscursiveImportError");
        }
        showImportDialog_ = false;
    }

    std::string exportResult;
    if (exportSelector_.draw(&showExportDialog_, exportResult, ".discursive.json", true)) {
        try {
            session_->saveDiscursiveSystemsToFile(exportResult);
            std::filesystem::path selected(exportResult);
            if (selected.has_parent_path()) {
                lastExportPath_ = selected.parent_path().string();
            }
            ImGui::OpenPopup("DiscursiveExportSuccess");
        } catch (const std::exception&) {
            ImGui::OpenPopup("DiscursiveExportError");
        }
        showExportDialog_ = false;
    }

    if (ImGui::BeginPopupModal("DiscursiveImportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Discursive systems imported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("DiscursiveImportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to import discursive systems.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("DiscursiveExportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Discursive systems exported successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("DiscursiveExportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to export discursive systems.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void DiscursiveSystemPanel::drawSystemList() {
    auto systems = session_->getDiscursiveSystemDTOs();

    if (systems.empty()) {
        ImGui::TextDisabled("No discursive systems registered.");
        return;
    }

    if (ImGui::BeginTable("DiscursiveSystemsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Sources");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Problems");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableSetupColumn("Effects");
        ImGui::TableHeadersRow();

        for (const auto& system : systems) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", system.id.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", system.sourceReferences.size());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", system.temporalContext.label.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%zu", system.declaredProblems.size());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%zu", system.declaredActions.size());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%zu", system.expectedEffects.size());
        }
        ImGui::EndTable();
    }
}

} // namespace UI::Panels
