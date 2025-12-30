#include "TimelinePanel.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "world3d/World3D.hpp"
#include "imgui.h"
#include <vector>

namespace UI::Panels {

void TimelinePanel::draw(bool* open) {
    if (!open || !*open) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Fourth Dimension (Resilience)", open)) {
        ImGui::End();
        return;
    }

    if (!trajectory_ || !vegPanel_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[!] Missing Dependencies");
        ImGui::End();
        return;
    }

    // Capture Controls
    if (ImGui::Button("Capture Current State", ImVec2(-1, 0))) {
        const auto* res = vegPanel_->getLastScenarioResult();
        if (res && !res->classification.empty()) {
            std::string meta = "State " + std::to_string(trajectory_->getNextOrdinal());
            
            // Dummy Water Mask for now (requires deeper integration with Hydro)
            std::vector<bool> waterMask; 
            
            Core::Domain::FourthDimension::TrajectoryService::captureState(
                *trajectory_,
                res->classification, // Indices
                vegPanel_->getSystem(), // System (for lookup)
                waterMask,
                meta
            );
        } else {
            // Warn
             ImGui::OpenPopup("CaptureFailed");
        }
    }

    if (ImGui::BeginPopup("CaptureFailed")) {
        ImGui::Text("Cannot capture state.");
        ImGui::Text("Please run 'Resolve Scenario' first.");
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();
    
    // Ghost Mode Warning
    if (ghostMode_) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 1.0f, 1.0f));
        ImGui::Text("[GHOST MODE ACTIVE]");
        ImGui::PopStyleColor();
        ImGui::TextWrapped("Viewing past state. Simulation is read-only.");
        
        if (ImGui::Button("Exit Ghost Mode (Restore Soil)")) {
            ghostMode_ = false;
            World3D::resetVisualization();
        }
        ImGui::Separator();
    }

    // Trajectory List
    ImGui::Text("Trajectory (%zu states)", trajectory_->getTimeSlices().size());
    
    ImGui::BeginChild("TrajectoryList", ImVec2(0, 0), true);
    auto& slices = trajectory_->getTimeSlices();
    
    for (int i = 0; i < (int)slices.size(); ++i) {
        const auto& slice = slices[i];
        
        std::string label = "T" + std::to_string(slice.getOrdinalIndex()) + ": " + slice.getMetadata();
        bool isSelected = (selectedSliceIndex_ == i);
        
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedSliceIndex_ = i;
            // Activate Ghost Mode immediately on selection?
            // Or require explicit "View"?
            // Let's do explicit View to avoid flashing.
        }
    }
    ImGui::EndChild();

    // Selected Item Actions
    if (selectedSliceIndex_ >= 0 && selectedSliceIndex_ < (int)slices.size()) {
        const auto& slice = slices[selectedSliceIndex_];
        ImGui::Separator();
        ImGui::Text("Selected: T%d", slice.getOrdinalIndex());
        
        if (ImGui::Button("View (Ghost)")) {
            ghostMode_ = true;
            applyGhostVisualization(slice);
        }
    }

    ImGui::End();
}

void TimelinePanel::applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice) {
    World3D::applyClassificationVisualization(slice.getEcologicalCoverState());
}

} // namespace UI::Panels
