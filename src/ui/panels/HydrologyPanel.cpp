#include "HydrologyPanel.hpp"
#include "imgui.h"

namespace UI::Panels {

void HydrologyPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    if (ImGui::Begin("Hydrology", open)) {
        if (ImGui::Button("Compute Drainage")) {
            lastDrainageStats_ = World3D::applyDrainageSimulation();
            hasDrainageStats_ = true;
            if (lastDrainageStats_.message.empty()) {
                showDrainage_ = true;
                showWatersheds_ = false;
                World3D::setDrainageVisualization(showDrainage_, showWatersheds_, showBasinOutlines_, drainageIntensity_);
                statusMessage_.clear();
            } else {
                statusMessage_ = lastDrainageStats_.message;
            }
        }

        if (!statusMessage_.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", statusMessage_.c_str());
        }

        ImGui::Separator();

        if (ImGui::Checkbox("Show Drainage (Flux)", &showDrainage_)) {
            if (showDrainage_) showWatersheds_ = false;
            World3D::setDrainageVisualization(showDrainage_, showWatersheds_, showBasinOutlines_, drainageIntensity_);
        }
        if (showDrainage_) {
            ImGui::Indent();
            if (ImGui::SliderFloat("Viz Threshold", &drainageIntensity_, 0.05f, 1.0f)) {
                World3D::setDrainageVisualization(showDrainage_, showWatersheds_, showBasinOutlines_, drainageIntensity_);
            }
            ImGui::Unindent();
        }

        if (ImGui::Checkbox("Show Watersheds (Basins)", &showWatersheds_)) {
            if (showWatersheds_) showDrainage_ = false;
            World3D::setDrainageVisualization(showDrainage_, showWatersheds_, showBasinOutlines_, drainageIntensity_);
        }
        if (showWatersheds_) {
            ImGui::Indent();
            if (ImGui::Checkbox("Show Basin Outlines", &showBasinOutlines_)) {
                World3D::setDrainageVisualization(showDrainage_, showWatersheds_, showBasinOutlines_, drainageIntensity_);
            }
            ImGui::Unindent();
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Legend:");
        ImGui::BulletText("Blue ramp: Flow accumulation");
        ImGui::BulletText("Dark lines: Basin boundaries");

        ImGui::Separator();

        if (hasDrainageStats_ && lastDrainageStats_.message.empty()) {
            ImGui::Text("Drainage Stats:");
            ImGui::Text("Max Accumulation: %d cells", lastDrainageStats_.maxAccumulation);
            ImGui::Text("Mean Accumulation: %.2f cells", lastDrainageStats_.meanAccumulation);
            ImGui::Text("River Cells (>50): %d", lastDrainageStats_.riverCells);
        } else {
            ImGui::TextDisabled("Drainage stats not available.");
        }

        ImGui::Separator();

        ImGui::Text("Hydrology Report");
        ImGui::SliderFloat("Stream Threshold (cells)", &streamThreshold_, 10.0f, 1000.0f, "%.0f");
        if (ImGui::Button("Update Hydrology Stats")) {
            lastHydrologyStats_ = World3D::getHydrologyStats(streamThreshold_);
            hasHydrologyStats_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Report...")) {
            showReportDialog_ = true;
            reportFileSelector_.Open("hydrology_report.txt");
        }

        if (hasHydrologyStats_) {
            ImGui::Text("Basins: %d (largest %.1f%%)", lastHydrologyStats_.basinCount, lastHydrologyStats_.largestBasinPct);
            ImGui::Text("Drainage Density: %.4e m-1", lastHydrologyStats_.drainageDensity);
            ImGui::Text("Max Flow Area: %.2f m2", lastHydrologyStats_.maxFlowAccumulation);
            ImGui::Text("Mean Slope: %.4f (%.1f%%)", lastHydrologyStats_.avgSlope, lastHydrologyStats_.avgSlope * 100.0f);
        } else {
            ImGui::TextDisabled("Hydrology stats not computed.");
        }

        if (showReportDialog_) {
            std::string path;
            if (reportFileSelector_.draw(&showReportDialog_, path, ".txt", true)) {
                bool ok = World3D::generateHydrologyReport(path, streamThreshold_);
                statusMessage_ = ok ? "Hydrology report saved." : "Failed to save hydrology report.";
                showReportDialog_ = false;
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
