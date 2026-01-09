#pragma once

#include "src/application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <imgui.h>
#include <string>
#include <vector>

namespace UI::Components {

/**
 * @brief A modal dialog to display AI interpretation results and allow saving them.
 */
class InterpretationModal {
public:
    static void Draw(const char* id, bool& isOpen, Application::DTO::Cognitive::InterpretationSnapshotDTO& snapshot, 
                    std::function<void(const Application::DTO::Cognitive::InterpretationSnapshotDTO&)> onSave) {
        
        if (!isOpen) return;

        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal(id, &isOpen)) {
            
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "COGNITIVE INTERPRETATION [%s]", snapshot.intent.c_str());
            ImGui::Separator();
            
            ImGui::Text("Context: %s", snapshot.inputContextSummary.c_str());
            ImGui::Text("Prompt Version: %s", snapshot.promptVersion.c_str());
            ImGui::Separator();

            ImGui::BeginChild("AIOutputChild", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), true);
            ImGui::TextWrapped("%s", snapshot.aiOutput.c_str());
            ImGui::EndChild();

            ImGui::Separator();
            
            if (ImGui::Button("Save to Epistemic Memory", ImVec2(240, 0))) {
                onSave(snapshot);
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Dismiss", ImVec2(120, 0))) {
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
};

} // namespace UI::Components
