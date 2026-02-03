#include "GlobalSynthesisPanel.hpp"
#include "imgui.h"
#include "src/ui/components/InterpretationModal.hpp"
#include "src/ui/components/InterpretationHistory.hpp"
#include "src/application/mappers/CognitiveMappers.hpp"
#include <algorithm>

namespace UI::Panels {

void GlobalSynthesisPanel::setSession(Application::Session* session) {
    session_ = session;
}

void GlobalSynthesisPanel::draw(bool* open) {
    if (!open || !*open) return;

    // Ensure AI Modal opens in main thread
    {
        std::lock_guard<std::mutex> lock(aiMutex_);
        if (aiResultReady_) {
            lastAiSnapshot_ = stagedAiSnapshot_;
            showAiModal_ = true;
            aiResultReady_ = false;
        }
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Strategic Global Synthesis (SGS)", open)) {
        
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
    ImGui::End();
}

void GlobalSynthesisPanel::drawAuditSection() {
    ImGui::TextWrapped("The Strategic Global Synthesis (SGS) evaluates the alignment between your Narrative Observations, "
                       "Discursive Systems, and Recommendation Trajectories.");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(aiRequestPending_ ? "Auditing Consistency..." : "Perform Strategic Consistency Audit", ImVec2(300, 50))) {
        if (session_) {
            aiRequestPending_ = true;
            
            // Collect all data
            auto narratives = session_->getNarrativeHistoryDTO();
            auto discursive = session_->getDiscursiveSystemDTOs();
            auto trajectory = session_->getRecommendationTrajectoryDTO();
            
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
                    aiResultReady_ = true;
                });
        }
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
