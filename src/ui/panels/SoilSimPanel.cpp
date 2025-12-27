#include "SoilSimPanel.hpp"
#include "world3d/World3D.hpp"
#include "core/domain/soils/SiBCS.hpp" // Added
#include <string>

namespace UI::Panels {

void SoilSimPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Soil Simulation (SCORPAN)", open)) {
        ImGui::TextWrapped("Configure environmental factors to predict SiBCS soil classes.");
        ImGui::Separator();

        // C - Climate
        ImGui::Text("Climate (C)");
        ImGui::SliderFloat("Rainfall (mm)", &params_.rainfall, 0.0f, 4000.0f);
        ImGui::SliderFloat("Temp (C)", &params_.temperature, -10.0f, 45.0f);
        
        ImGui::Separator();

        // O - Organisms
        ImGui::Text("Organisms (O)");
        ImGui::SliderFloat("Vegetation Density", &params_.vegetationDensity, 0.0f, 1.0f);

        ImGui::Separator();

        // A - Age
        ImGui::Text("Age (A)");
        ImGui::SliderFloat("Time Factor", &params_.ageFactor, 0.0f, 1.0f, "Young -> Old");

        ImGui::Separator();

        // P - Parent Material
        ImGui::Text("Parent Material (P)");
        const char* items[] = { "Igneous", "Sedimentary", "Metamorphic" };
        static int currentItem = 1; // Sedimentary default
        if (ImGui::Combo("Material", &currentItem, items, IM_ARRAYSIZE(items))) {
            params_.parentMaterial = static_cast<Core::Domain::Soils::ParentMaterialType>(currentItem);
        }

        ImGui::Separator();
        
        ImGui::Text("Visualization Level (SiBCS)");
        const char* levels[] = { "1. Order (Ordem)", "2. Suborder (Subordem)", "3. Great Group (Grande Grupo)" };
        // We use 0-based index for ImGui, but 1-based for logic
        int currentLevelIndex = visualizationLevel_ - 1;
        if (ImGui::Combo("Level", &currentLevelIndex, levels, IM_ARRAYSIZE(levels))) {
            visualizationLevel_ = currentLevelIndex + 1;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Run Simulation", ImVec2(-1, 40))) {
            World3D::applySoilSimulation(params_, visualizationLevel_);
        }

        ImGui::Separator();
        ImGui::Text("Legend (Colors)");
        ImGui::Spacing();

        // Universal Legend Logic using Vector Vectors (getCommonVectors)
        // This handles all levels (1, 2, 3...) elegantly
        auto commonSoils = Core::Domain::Soils::SiBCSHelper::getCommonVectors(visualizationLevel_);
        
        for (const auto& soil : commonSoils) {
            glm::vec3 c = Core::Domain::Soils::SiBCSHelper::getColor(soil, visualizationLevel_);
            std::string name = Core::Domain::Soils::SiBCSHelper::getName(soil, visualizationLevel_);
            
            ImGui::ColorButton("##c", ImVec4(c.r, c.g, c.b, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker, ImVec2(20, 20));
            ImGui::SameLine();
            ImGui::Text("%s", name.c_str());
        }
        
    }
    ImGui::End();
}

} // namespace UI::Panels
