#pragma once

#include "src/application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <imgui.h>
#include <vector>
#include <functional>
#include <string>

namespace UI::Components {

/**
 * @brief Reusable component to list and view Interpretation Snapshots.
 */
class InterpretationHistory {
public:
    static void Draw(const std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO>& snapshots) {
        if (snapshots.empty()) {
            ImGui::TextDisabled("No interpretation history available.");
            return;
        }

        static int selectedIndex = -1;
        static bool showDetailModal = false;

        // Table
        if (ImGui::BeginTable("HistoryTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Context", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Intent", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            bool triggerOpen = false;
            for (int i = 0; i < snapshots.size(); ++i) {
                const auto& snap = snapshots[i];
                ImGui::TableNextRow();
                
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", snap.snapshotId.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", snap.inputContextSummary.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", snap.intent.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(i);
                if (ImGui::Button("View")) {
                    selectedIndex = i;
                    showDetailModal = true;
                    triggerOpen = true;
                    // Defer OpenPopup to avoid ID stack mismatch
                }
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (triggerOpen) {
                ImGui::OpenPopup("HistoryDetailModal");
            }
        }


        // Modal logic continues below

        if (showDetailModal && selectedIndex >= 0 && selectedIndex < snapshots.size()) {
            // Ensure Popup is open (must be called in the same scope context as checking condition if outside loop, but here we triggered it inside)
            // Actually, OpenPopup works across frames, so we need to BeginPopupModal here.
            
            // To ensure it works robustly, we keep OpenPopup logic near the button (above) and just draw the modal here.
            
            ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
            if (ImGui::BeginPopupModal("HistoryDetailModal", &showDetailModal)) {
                const auto& snap = snapshots[selectedIndex];

                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "SNAPSHOT: %s", snap.snapshotId.c_str());
                ImGui::Separator();
                
                ImGui::TextDisabled("Intent: %s", snap.intent.c_str());
                ImGui::TextDisabled("Context: %s", snap.inputContextSummary.c_str());
                ImGui::TextDisabled("Source Bundle: %s", snap.sourceBundleId.c_str());
                
                ImGui::Separator();
                ImGui::BeginChild("ContentScroll", ImVec2(0, -40));
                ImGui::TextWrapped("%s", snap.aiOutput.c_str());
                ImGui::EndChild();
                
                ImGui::Separator();
                if (ImGui::Button("Close", ImVec2(120, 0))) {
                    showDetailModal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
};

} // namespace UI::Components
