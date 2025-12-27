#include "TerrainGenPanel.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp"

namespace UI::Panels {

void TerrainGenPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    if (ImGui::Begin("Generate Terrain", open)) {
        static char genFilename[256] = "assets/data/generated_terrain.obj";
        static int genWidth = 100;
        static int genHeight = 100;
        static float genSpacing = 1.0f;
        static int genType = 0;
        const char* typeItems[] = { "Hills", "Mountains", "Flat", "Canyon" };

        ImGui::InputText("Output Filename", genFilename, IM_ARRAYSIZE(genFilename));
        ImGui::InputInt("Width", &genWidth);
        ImGui::InputInt("Height", &genHeight);
        ImGui::InputFloat("Spacing (m)", &genSpacing);
        ImGui::Combo("Pattern", &genType, typeItems, IM_ARRAYSIZE(typeItems));
        
        // Validate inputs
        if (genWidth < 10) genWidth = 10;
        if (genHeight < 10) genHeight = 10;
        if (genSpacing < 0.1f) genSpacing = 0.1f;
        
        ImGui::Separator();

        bool isGen = World3D::isTerrainGenerating();
        if (isGen) {
            ImGui::Text("Generating... Please wait.");
        } else {
            if (ImGui::Button("Generate & Load")) {
                if (World3D::generateTerrain(genFilename, genWidth, genHeight, genSpacing, genType, true)) {
                    // Load handled automatically by Engine on success
                }
            }
            
            ImGui::SameLine();
                
                if (ImGui::Button("Generate Only")) {
                if (World3D::generateTerrain(genFilename, genWidth, genHeight, genSpacing, genType, false)) {
                        ImGui::OpenPopup("GenSuccess");
                }
                }
        }

            if (ImGui::BeginPopupModal("GenSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Terrain Generation Started!");
                if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); *open = false; }
                ImGui::EndPopup();
            }
    }
    ImGui::End();
}

} // namespace UI::Panels
