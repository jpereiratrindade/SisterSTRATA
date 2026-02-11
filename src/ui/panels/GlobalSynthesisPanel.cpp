#include "GlobalSynthesisPanel.hpp"
#include "imgui.h"
#include "src/ui/components/InterpretationModal.hpp"
#include "src/ui/components/InterpretationHistory.hpp"
#include "src/application/mappers/CognitiveMappers.hpp"
#include <algorithm>
#include <cstring>

namespace UI::Panels {

void GlobalSynthesisPanel::setSession(Application::Session* session) {
    session_ = session;
}

void GlobalSynthesisPanel::drawTabContent(bool shouldOpenAiPopup) {
    if (!session_) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: No Session Connected");
        return;
    }

    if (shouldOpenAiPopup) {
        ImGui::OpenPopup("GlobalSynthesisAIResult");
    }
    
    if (ImGui::BeginTabBar("SGSTabs")) {
        if (ImGui::BeginTabItem("Strategic Audit")) {
            drawAuditSection();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Synthesis Memory")) {
            drawHistoryTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // AI Interpretation Modal
    if (showAiModal_) {
        UI::Components::InterpretationModal::Draw("GlobalSynthesisAIResult", showAiModal_, lastAiSnapshot_, [this](const auto& snapshot){
            if (session_) session_->saveInterpretationSnapshotDTO(snapshot);
        });
    }
}

void GlobalSynthesisPanel::drawInline(const char* idSuffix) {
    bool shouldOpenAiPopup = false;
    // Ensure AI Modal opens in main thread
    {
        std::lock_guard<std::mutex> lock(aiMutex_);
        if (aiResultReady_) {
            lastAiSnapshot_ = stagedAiSnapshot_;
            showAiModal_ = true;
            shouldOpenAiPopup = true;
            aiResultReady_ = false;
        }
    }

    ImGui::PushID(idSuffix ? idSuffix : "WorkspaceGlobalSynthesis");
    drawTabContent(shouldOpenAiPopup);
    ImGui::PopID();
}

void GlobalSynthesisPanel::draw(bool* open) {
    if (!open || !*open) return;

    bool shouldOpenAiPopup = false;
    // Ensure AI Modal opens in main thread
    {
        std::lock_guard<std::mutex> lock(aiMutex_);
        if (aiResultReady_) {
            lastAiSnapshot_ = stagedAiSnapshot_;
            showAiModal_ = true;
            shouldOpenAiPopup = true;
            aiResultReady_ = false;
        }
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Strategic Global Synthesis (SGS)", open)) {
        ImGui::PushID("StandaloneGlobalSynthesis");
        drawTabContent(shouldOpenAiPopup);
        ImGui::PopID();
    }
    ImGui::End();
}

void GlobalSynthesisPanel::drawAuditSection() {
    ImGui::TextWrapped("The Strategic Global Synthesis (SGS) evaluates the alignment between your Narrative Observations, "
                       "Discursive Systems, and Recommendation Trajectories.");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (aiRequestPending_) {
        ImGui::BeginDisabled();
        ImGui::Button("Auditing Consistency...", ImVec2(300, 50));
        ImGui::EndDisabled();
    } else if (ImGui::Button("Perform Strategic Consistency Audit", ImVec2(300, 50))) {
        if (session_) {
            // Collect all data
            auto narratives = session_->getNarrativeHistoryDTO();
            auto discursive = session_->getDiscursiveSystemDTOs();
            auto trajectory = session_->getRecommendationTrajectoryDTO();

            if (narratives.empty() && discursive.empty() && trajectory.snapshots.empty()) {
                ImGui::OpenPopup("SGSNoDataPopup");
                return;
            }

            if (!session_->getLLMService()) {
                ImGui::OpenPopup("SGSNoLLMPopup");
                return;
            }

            aiRequestPending_ = true;
            lastAuditFailed_ = false;
            auditStatus_ = "SGS audit started. Waiting for model response...";

            auto bundle = Application::Mappers::Cognitive::createBundle(
                "global_audit", 
                narratives, 
                discursive, 
                &trajectory,
                "" // getSummary() not available on raw trajectory yet
            );
            
            // Inject Analytical Profile for context
            bundle.trajectoryImpactProfile = session_->generateImpactProfileText();

            session_->requestAIInterpretation(bundle, 
                Application::Services::Cognitive::InterpretationMode::GlobalSynthesis,
                [this](const auto& snapshot) {
                    std::lock_guard<std::mutex> lock(aiMutex_);
                    stagedAiSnapshot_ = snapshot;
                    aiRequestPending_ = false;
                    lastAuditFailed_ = snapshot.aiOutput.rfind("Error:", 0) == 0;
                    auditStatus_ = lastAuditFailed_
                        ? "SGS audit failed. See interpretation output for details."
                        : "SGS audit completed. Interpretation is available.";
                    aiResultReady_ = true;
                });
        }
    }

    if (ImGui::BeginPopupModal("SGSNoDataPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.0f, 1.0f), "No data available for SGS.");
        ImGui::TextWrapped("Load or ingest at least one Narrative, Discursive, or Recommendation record before running the audit.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("SGSNoLLMPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.35f, 1.0f), "LLM service is not configured.");
        ImGui::TextWrapped("Check Ollama availability or session LLM setup before running SGS.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (!auditStatus_.empty()) {
        const ImVec4 okColor(0.45f, 0.9f, 0.55f, 1.0f);
        const ImVec4 failColor(0.95f, 0.45f, 0.35f, 1.0f);
        ImGui::Spacing();
        ImGui::TextColored(lastAuditFailed_ ? failColor : okColor, "%s", auditStatus_.c_str());
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Note: This process analyzes the entire horizontal context of the project.");
}

void GlobalSynthesisPanel::drawHistoryTab() {
    if (session_) {
        auto snapshots = session_->getInterpretationSnapshots();
        std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> filtered;

        // Filter for Global Synthesis context
        std::copy_if(snapshots.begin(), snapshots.end(), std::back_inserter(filtered), [](const auto& s){
            return s.intent == "global_synthesis";
        });

        std::reverse(filtered.begin(), filtered.end());
        UI::Components::InterpretationHistory::Draw(filtered);
    }
}

} // namespace UI::Panels
