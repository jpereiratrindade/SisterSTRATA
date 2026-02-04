#include "src/ui/panels/DiscursiveSystemPanel.hpp"
#include "imgui.h"
#include <cstring>
#include <filesystem>
#include "application/mappers/CognitiveMappers.hpp"
#include "ui/components/InterpretationModal.hpp"
#include "ui/components/InterpretationHistory.hpp"
#include <vector>
#include <algorithm>

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
            
            if (ImGui::BeginTabItem("Ingestion", nullptr, targetTab_ == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {
                // Check for Deferred AI Results (Thread-safe UI update) inside the tab scope
                {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    if (aiResultReady_) {
                        lastAiSnapshot_ = stagedAiSnapshot_; // Safe copy to UI thread
                        
                        // Check for error in output
                        if (lastAiSnapshot_.aiOutput.rfind("Error:", 0) == 0) {
                            ImGui::OpenPopup("AIError");
                        } else {
                            ImGui::OpenPopup("AI Discursive Proposal");
                        }
                        aiResultReady_ = false;
                    }
                }

                drawIngestionForm();
                
                // AI Result Modal (Matches the ID used in OpenPopup inside tab)
                UI::Components::InterpretationModal::Draw("AI Discursive Proposal", showAiModal_, lastAiSnapshot_, [this](const auto& snap) {
                    session_->saveInterpretationSnapshotDTO(snap);
                });

                if (ImGui::BeginPopupModal("AIError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "AI Analysis Failed");
                    ImGui::Separator();
                    ImGui::TextWrapped("%s", lastAiSnapshot_.aiOutput.c_str());
                    if (ImGui::Button("Close")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::Separator();
                drawSystemList();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Registered History", nullptr, targetTab_ == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {
                drawSystemList();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Epistemic Memory", nullptr, targetTab_ == 2 ? ImGuiTabItemFlags_SetSelected : 0)) {
                auto snapshots = session_->getInterpretationSnapshots();
                std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> filtered;

                // Filter for Discursive Context
                std::copy_if(snapshots.begin(), snapshots.end(), std::back_inserter(filtered), [](const auto& s){
                    return s.intent == "discursive_draft" || s.intent == "coherence_check";
                });

                std::reverse(filtered.begin(), filtered.end());
                UI::Components::InterpretationHistory::Draw(filtered);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();

            targetTab_ = -1; // Reset selection
        }
    }
    ImGui::End();
}

void DiscursiveSystemPanel::drawIngestionForm() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "SYSTEM INGESTION (DSC)");
    ImGui::SameLine(ImGui::GetWindowWidth() - 500);
    
    if (ImGui::Button(aiRequestPending_ ? "Waiting..." : "Ask Qwen to Propose System", ImVec2(240, 60))) {
        auto systems = session_->getDiscursiveSystemDTOs();
        if (!systems.empty()) {
            try {
                // Pre-validation: Ensure discursive systems are valid
                bool validSources = std::all_of(systems.begin(), systems.end(), [](const auto& s){ return !s.id.empty(); });
                
                if (validSources) {
                    aiRequestPending_ = true;
                    auto bundle = Application::Mappers::Cognitive::createBundle("discursive_draft", {}, systems);
                    
                    // Inject Analytical Profile
                    bundle.trajectoryImpactProfile = session_->generateImpactProfileText();

                    session_->requestAIInterpretation(bundle, 
                        Application::Services::Cognitive::InterpretationMode::DiscursiveDraft,
                        [this](const auto& snapshot) {
                            std::lock_guard<std::mutex> lock(aiMutex_);
                            stagedAiSnapshot_ = snapshot;
                            showAiModal_ = true;
                            aiRequestPending_ = false;
                            aiResultReady_ = true; // Signal the main thread to open the popup
                        });
                }
            } catch (const std::exception& e) {
                // Fallback / Log error
                aiRequestPending_ = false;
                ImGui::OpenPopup("AIAnalysisError"); 
            }
        } else {
            ImGui::OpenPopup("DiscursiveContextEmptyError");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(aiRequestPending_ ? "Waiting..." : "Evaluate Logical Coherence", ImVec2(240, 60))) {
        auto systems = session_->getDiscursiveSystemDTOs();
        if (!systems.empty()) {
            try {
                aiRequestPending_ = true;
                auto bundle = Application::Mappers::Cognitive::createBundle("system_evaluation", {}, systems);
                
                // Inject Analytical Profile for context
                bundle.trajectoryImpactProfile = session_->generateImpactProfileText();

                session_->requestAIInterpretation(bundle, 
                    Application::Services::Cognitive::InterpretationMode::CoherenceCheck,
                    [this](const auto& snapshot) {
                        std::lock_guard<std::mutex> lock(aiMutex_);
                        stagedAiSnapshot_ = snapshot;
                        showAiModal_ = true;
                        aiRequestPending_ = false;
                        aiResultReady_ = true;
                    });
            } catch (...) {
                aiRequestPending_ = false;
                ImGui::OpenPopup("AIAnalysisError");
            }
        } else {
            ImGui::OpenPopup("SystemsEmptyError");
        }
    }
    
    if (ImGui::BeginPopupModal("DiscursiveContextEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Discursive Systems Found");
        ImGui::Text("The 'Propose System' tool uses registered Discursive Systems for synthesis.");
        ImGui::Text("Please load an example or add a system manually first.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("SystemsEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Systems Found");
        ImGui::Text("To evaluate coherence, you need at least one Discursive System registered.");
        ImGui::Text("Please load an example or add a system manually first.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    
    // Catch-all error popup for try-catch failures
    if (ImGui::BeginPopupModal("AIAnalysisError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
         ImGui::TextColored(ImVec4(1,0,0,1), "Error Preparing Data for AI");
         ImGui::Text("Invalid narratives or data corruption.");
         if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
         ImGui::EndPopup();
    }

    ImGui::Separator();
    
    // Impact Profile Preview
    if (ImGui::CollapsingHeader("Live Trajectory Impact Analysis")) {
        // Simulation Controls
        ImGui::TextDisabled("Simulation Tools (Debug)");
        if (ImGui::Button("Simulate: Stability")) {
            session_->simulateCondition(Application::Session::SimulationType::Stability);
        }
        ImGui::SameLine();
        if (ImGui::Button("Simulate: Fragmentation")) {
            session_->simulateCondition(Application::Session::SimulationType::Fragmentation);
        }
        ImGui::SameLine();
        if (ImGui::Button("Simulate: Deforestation")) {
            session_->simulateCondition(Application::Session::SimulationType::Deforestation);
        }
        ImGui::Separator();

        std::string profileText = session_->generateImpactProfileText();
        if (profileText.empty()) {
             ImGui::TextDisabled("No sufficient trajectory data for analysis.");
        } else {
             ImGui::TextWrapped("%s", profileText.c_str());
        }
    }

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

    if (isEditing_) {
        ImGui::TextColored(ImVec4(1,1,0,1), "EDITING MODE: %s", editingId_.c_str());
        if (ImGui::Button("Cancel Edit")) {
            isEditing_ = false;
            editingId_.clear();
            // Clear form
            inputSystemId_[0] = '\0';
            declaredProblems_.clear();
            declaredActions_.clear();
            allegedMechanisms_.clear();
            expectedEffects_.clear();
            metadata_.clear();
            sourceReferences_.clear();
        }
        ImGui::SameLine();
    }

    if (ImGui::Button(isEditing_ ? "Save Changes" : "Register Discursive System", ImVec2(240, 0))) {
        Application::DTO::DiscursiveSystemDTO dto;

        if (strlen(inputSystemId_)) {
             dto.id = inputSystemId_;
        } else {
             dto.id = "DS-" + std::to_string(session_->getDiscursiveSystemCount() + 1);
        }

        // Copy lists
        dto.declaredProblems = declaredProblems_;
        dto.declaredActions = declaredActions_;
        dto.allegedMechanisms = allegedMechanisms_;
        dto.expectedEffects = expectedEffects_;
        
        // --- Helper: Auto-add pending text in buffers ---
        if (strlen(inputProblem_) > 0) dto.declaredProblems.push_back(inputProblem_);
        if (strlen(inputAction_) > 0) dto.declaredActions.push_back(inputAction_);
        if (strlen(inputMechanism_) > 0) dto.allegedMechanisms.push_back(inputMechanism_);
        if (strlen(inputEffect_) > 0) dto.expectedEffects.push_back(inputEffect_);

        // Validation: Prevent empty records
        bool hasContent = !dto.declaredProblems.empty() || 
                          !dto.declaredActions.empty() || 
                          !dto.allegedMechanisms.empty() || 
                          !dto.expectedEffects.empty();

        if (!hasContent && !isEditing_) { // Allow editing to clear stuff? Maybe not. Let's block both for now context-wise.
             ImGui::OpenPopup("DiscursiveEmptyError");
        } 
        else {
            dto.temporalContext = Application::DTO::TemporalContextDTO{
                TEMPORAL_VALUES[inputTemporalCategory_],
                inputTemporalLabel_
            };
            dto.interpretationMetadata = metadata_; // Metadata is strictly optional

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
                if (isEditing_) {
                    session_->updateDiscursiveSystemDTO(editingId_, dto);
                    ImGui::OpenPopup("DiscursiveUpdateSuccess");
                    isEditing_ = false;
                    editingId_.clear();
                } else {
                    session_->registerDiscursiveSystemDTO(dto);
                    ImGui::OpenPopup("DiscursiveSuccess");
                }
                // Clear form
                declaredProblems_.clear();
                declaredActions_.clear();
                allegedMechanisms_.clear();
                expectedEffects_.clear();
                metadata_.clear();
                inputSystemId_[0] = '\0';
                sourceReferences_.clear();
                // Clear buffers too if we auto-added them
                inputProblem_[0] = '\0'; 
                inputAction_[0] = '\0';
                inputMechanism_[0] = '\0';
                inputEffect_[0] = '\0';
            } catch (const std::exception& e) {
                ImGui::OpenPopup("DiscursiveError");
            }
        }
    }

    if (ImGui::BeginPopupModal("DiscursiveSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Discursive system registered successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("DiscursiveUpdateSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Discursive system updated successfully.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("DiscursiveEmptyError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Cannot register empty system!");
        ImGui::Text("Please add at least one Problem, Action, Mechanism, or Effect.");
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

    if (ImGui::BeginTable("DiscursiveSystemsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Actions");
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
            ImGui::PushID(system.id.c_str());
            if (ImGui::Button("Edit")) {
                loadIntoForm(system);
                // Hint user to switch tab
                // In a more complex UI we would programmatically switch tabs
            }
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                session_->removeDiscursiveSystemDTO(system.id);
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", system.id.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%zu", system.sourceReferences.size());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", system.temporalContext.label.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%zu", system.declaredProblems.size());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%zu", system.declaredActions.size());

            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%zu", system.expectedEffects.size());
        }
        ImGui::EndTable();
    }
}

void DiscursiveSystemPanel::loadIntoForm(const Application::DTO::DiscursiveSystemDTO& dto) {
    isEditing_ = true;
    editingId_ = dto.id;
    targetTab_ = 0; // Return to Ingestion tab for editing
    
    // Copy ID
    strncpy(inputSystemId_, dto.id.c_str(), sizeof(inputSystemId_) - 1);

    // Copy Vector fields
    declaredProblems_ = dto.declaredProblems;
    declaredActions_ = dto.declaredActions;
    allegedMechanisms_ = dto.allegedMechanisms;
    expectedEffects_ = dto.expectedEffects;
    metadata_ = dto.interpretationMetadata;
    sourceReferences_ = dto.sourceReferences;

    // Time
    strncpy(inputTemporalLabel_, dto.temporalContext.label.c_str(), sizeof(inputTemporalLabel_) - 1);
    
    // Find category index if possible (simple matching)
    for (int i = 0; i < IM_ARRAYSIZE(TEMPORAL_VALUES); i++) {
        if (dto.temporalContext.category == TEMPORAL_VALUES[i]) {
            inputTemporalCategory_ = i;
            break;
        }
    }

    // Sources Input (optional, maybe clear or load first one)
    inputSourceId_[0] = '\0';
    inputSourceDate_[0] = '\0';
    inputSourceAuthor_[0] = '\0';
}

} // namespace UI::Panels
