#include "SoilSimPanel.hpp"
#include "world3d/World3D.hpp"
#include "core/domain/soils/SiBCS.hpp" // Added
#include <string>

namespace UI::Panels {

void SoilSimPanel::drawScorpan(bool* open) {
    if (!open || !(*open)) return;

    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("SCORPAN Inputs", open)) {
        ImGui::TextWrapped("Configure environmental factors (Inputs).");
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
    }
    ImGui::End();
}

void SoilSimPanel::drawSiBCS(bool* open) {
    if (!open || !(*open)) return;

    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("SiBCS Output (Simulation)", open)) {
        ImGui::TextWrapped("Visualization and Classification (Outputs).");
        ImGui::Separator();
        
        // Execution Button
        if (ImGui::Button("Update Simulation", ImVec2(-1, 40))) {
            World3D::applySoilSimulation(params_, visualizationLevel_, filter_);
        }
        
        ImGui::Separator();
        
        ImGui::Text("Visualization Level (Colors)");
        const char* levels[] = { 
            "1. Order (Ordem)", 
            "2. Suborder (Subordem)", 
            "3. Great Group (Grande Grupo)",
            "4. Subgroup (Subgrupo)",
            "5. Family (Família)",
            "6. Series (Série)"
        };
        int currentLevelIndex = visualizationLevel_ - 1;
        if (ImGui::Combo("Level", &currentLevelIndex, levels, IM_ARRAYSIZE(levels))) {
            visualizationLevel_ = currentLevelIndex + 1;
        }

        ImGui::Separator();
        ImGui::Text("Filters (Multi-Select)");
        
        // Filter 1: Orders
        if (ImGui::BeginCombo("Orders", "Select Allowed...")) {
            for (auto order : Core::Domain::Soils::SiBCSHelper::getAllOrders()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedOrders) if (allowed == order) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(order).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) { // Remove
                        auto it = std::remove(filter_.allowedOrders.begin(), filter_.allowedOrders.end(), order);
                        filter_.allowedOrders.erase(it, filter_.allowedOrders.end());
                    } else { // Add
                        filter_.allowedOrders.push_back(order);
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Filter 2: Suborders
        if (ImGui::BeginCombo("Suborders", "Select Allowed...")) {
            for (auto sub : Core::Domain::Soils::SiBCSHelper::getAllSuborders()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedSuborders) if (allowed == sub) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(sub).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) {
                        auto it = std::remove(filter_.allowedSuborders.begin(), filter_.allowedSuborders.end(), sub);
                        filter_.allowedSuborders.erase(it, filter_.allowedSuborders.end());
                    } else {
                        filter_.allowedSuborders.push_back(sub);
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Filter 3: Great Groups
        if (ImGui::BeginCombo("Gr. Groups", "Select Allowed...")) {
            for (auto group : Core::Domain::Soils::SiBCSHelper::getAllGreatGroups()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedGreatGroups) if (allowed == group) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(group).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) {
                        auto it = std::remove(filter_.allowedGreatGroups.begin(), filter_.allowedGreatGroups.end(), group);
                        filter_.allowedGreatGroups.erase(it, filter_.allowedGreatGroups.end());
                    } else {
                        filter_.allowedGreatGroups.push_back(group);
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Filter 4: Subgroups
        if (ImGui::BeginCombo("Subgroups", "Select Allowed...")) {
            for (auto sub : Core::Domain::Soils::SiBCSHelper::getAllSubgroups()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedSubgroups) if (allowed == sub) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(sub).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) {
                        auto it = std::remove(filter_.allowedSubgroups.begin(), filter_.allowedSubgroups.end(), sub);
                        filter_.allowedSubgroups.erase(it, filter_.allowedSubgroups.end());
                    } else {
                        filter_.allowedSubgroups.push_back(sub);
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Filter 5: Families
        if (ImGui::BeginCombo("Families", "Select Allowed...")) {
            for (auto fam : Core::Domain::Soils::SiBCSHelper::getAllFamilies()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedFamilies) if (allowed == fam) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(fam).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) {
                        auto it = std::remove(filter_.allowedFamilies.begin(), filter_.allowedFamilies.end(), fam);
                        filter_.allowedFamilies.erase(it, filter_.allowedFamilies.end());
                    } else {
                        filter_.allowedFamilies.push_back(fam);
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Filter 6: Series
        if (ImGui::BeginCombo("Series", "Select Allowed...")) {
            for (auto ser : Core::Domain::Soils::SiBCSHelper::getAllSeries()) {
                bool isSelected = false;
                for (auto allowed : filter_.allowedSeries) if (allowed == ser) isSelected = true;
                
                if (ImGui::Selectable(Core::Domain::Soils::SiBCSHelper::getBaseName(ser).c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    if (isSelected) {
                        auto it = std::remove(filter_.allowedSeries.begin(), filter_.allowedSeries.end(), ser);
                        filter_.allowedSeries.erase(it, filter_.allowedSeries.end());
                    } else {
                        filter_.allowedSeries.push_back(ser);
                    }
                }
            }
            ImGui::EndCombo();
        }
        
        if (ImGui::Button("Clear Filters")) {
            filter_.allowedOrders.clear();
            filter_.allowedSuborders.clear();
            filter_.allowedGreatGroups.clear();
            filter_.allowedSubgroups.clear();
            filter_.allowedFamilies.clear();
            filter_.allowedSeries.clear();
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Legend (Colors)");
        ImGui::Spacing();

        // Universal Legend Logic 
        auto commonSoils = Core::Domain::Soils::SiBCSHelper::getCommonVectors(visualizationLevel_);
        
        ImGui::BeginChild("LegendScroll", ImVec2(0, 0), false, 0); // Scrollable area for legend
        for (const auto& soil : commonSoils) {
            glm::vec3 c = Core::Domain::Soils::SiBCSHelper::getColor(soil, visualizationLevel_);
            std::string name = Core::Domain::Soils::SiBCSHelper::getName(soil, visualizationLevel_);
            
            ImGui::ColorButton("##c", ImVec4(c.r, c.g, c.b, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker, ImVec2(20, 20));
            ImGui::SameLine();
            ImGui::TextWrapped("%s", name.c_str());
        }
        ImGui::EndChild();
        
    }
    ImGui::End();
}

} // namespace UI::Panels
