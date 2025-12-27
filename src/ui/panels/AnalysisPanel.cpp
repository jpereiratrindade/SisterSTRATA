#include "AnalysisPanel.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp"
#include <cstdio> // for snprintf if needed

namespace UI::Panels {

void AnalysisPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    if (ImGui::Begin("Analysis Report", open)) {
        auto stats = World3D::getSlopeAnalysisStats();
        
        if (stats.total > 0) {
            ImGui::Text("Total Vertices: %d", stats.total);
            ImGui::Separator();
            
            auto drawRow = [&](const char* label, int count, ImVec4 color) {
                float pct = (float)count / (float)stats.total * 100.0f;
                ImGui::TextColored(color, "%s: %d (%.1f%%)", label, count, pct);
            };

            drawRow("Flat (0-5 deg)", stats.countFlat, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            drawRow("Gentle (5-20 deg)", stats.countGentle, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
            drawRow("Moderate (20-45 deg)", stats.countModerate, ImVec4(0.9f, 0.5f, 0.0f, 1.0f));
            drawRow("Steep (>45 deg)", stats.countSteep, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
            
            ImGui::Separator();
            
            static char reportFilename[128] = "slope_report.txt";
            ImGui::InputText("Filename", reportFilename, IM_ARRAYSIZE(reportFilename));
            
            if (ImGui::Button("Save Report")) {
                if (World3D::saveReport(reportFilename)) {
                    ImGui::OpenPopup("Saved");
                }
            }
            
            if (ImGui::BeginPopupModal("Saved", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Report saved successfully!");
                if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                *open = false;
            }
        } else {
            ImGui::Text("No data analyzed.");
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
