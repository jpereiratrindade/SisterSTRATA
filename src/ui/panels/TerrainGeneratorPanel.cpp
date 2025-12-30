/**
 * @file TerrainGeneratorPanel.cpp
 * @brief Implementation of the procedural terrain generation and export UI.
 */
#include "TerrainGeneratorPanel.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp"
#include <filesystem>

namespace UI::Panels {

TerrainGeneratorPanel::TerrainGeneratorPanel() {
    // defaults
}

void TerrainGeneratorPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    // Center window initially
    ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Terrain Simulator", open)) {
        ImGui::TextWrapped("Procedural Terrain Generator & Exporter");
        ImGui::Separator();

        ImGui::Text("Grid Settings");
        ImGui::DragInt("Width (cells)", &width_, 1, 16, 4096);
        ImGui::DragInt("Height (cells)", &height_, 1, 16, 4096);
        ImGui::DragFloat("Spacing (m)", &spacing_, 0.1f, 0.1f, 100.0f);

        ImGui::Separator();
        ImGui::Text("Pattern");
        const char* types[] = { "Flat", "Hills", "Mountains", "Canyon", "Showcase (Complete)" };
        ImGui::Combo("Type", &selectedType_, types, IM_ARRAYSIZE(types));

        ImGui::Separator();
        ImGui::Text("Export Options");
        ImGui::InputText("Output Filename", filenameBuffer_, sizeof(filenameBuffer_));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Supported extensions: .csv (Drainage), .obj (Visual)");
        }
        ImGui::Checkbox("Auto-Load after Generation", &autoLoad_);

        ImGui::Separator();

        if (World3D::isTerrainGenerating()) {
            float progress = World3D::getGenerationProgress();
            std::string msg = World3D::getGenerationMessage();
            ImGui::ProgressBar(progress, ImVec2(-1, 0), msg.c_str());
            ImGui::TextDisabled("Generating... Please wait.");
        } else {
            if (ImGui::Button("Generate & Export", ImVec2(-1, 40))) {
                // Determine logic based on extension
                std::string path = filenameBuffer_;
                // Validate path (basic)
                if (path.empty()) path = "terrain.csv";
                
                // Add extension if missing? No, user should specify.
                
                // Call Generator
                World3D::generateTerrain(path, width_, height_, spacing_, selectedType_, autoLoad_);
            }
        }
        
        ImGui::TextDisabled("Note: Use .csv for Hydrology/Drainage support.");
    }
    ImGui::End();
}

} // namespace UI::Panels
