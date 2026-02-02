#include "SoilSimPanel.hpp"
#include "application/services/World3DService.hpp"
#include "application/services/SoilAnalysisService.hpp"
#include "ui/panels/PatchAnalysisPanel.hpp"
#include <string>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace UI::Panels {

void SoilSimPanel::drawScorpan(bool* open) {
    if (!open || !(*open)) return;

    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("SCORPAN Inputs", open)) {
        ImGui::TextWrapped("Configure environmental factors (Inputs).");
        ImGui::Separator();

        // Presets
        const char* presets[] = { "Custom", "Tropical Mature (All Soils)", "Young/Mountanous", "Arid" };
        static int currentPreset = 0;
        int presetCount = static_cast<int>(sizeof(presets) / sizeof(presets[0]));
        if (ImGui::Combo("Presets", &currentPreset, presets, presetCount)) {
            if (currentPreset == 1) { // Tropical Mature
                params_.rainfall = 2500.0f;
                params_.temperature = 28.0f;
                params_.ageFactor = 0.85f; // Ensures Latossolos (>0.7)
                params_.vegetationDensity = 0.9f;
                params_.parentMaterial = 1; // Sedimentary
            } else if (currentPreset == 2) { // Young
                params_.rainfall = 1800.0f;
                params_.temperature = 22.0f;
                params_.ageFactor = 0.2f; // Only young soils (Cambissolos, Neossolos)
                params_.vegetationDensity = 0.4f;
                params_.parentMaterial = 0; // Igneous
            } else if (currentPreset == 3) { // Arid
                params_.rainfall = 400.0f;
                params_.temperature = 35.0f;
                params_.ageFactor = 0.6f;
                params_.vegetationDensity = 0.1f;
            }
        }
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
        const auto materials = Application::Services::SoilAnalysisService::getParentMaterials();
        static int currentItem = 1; // Sedimentary default
        if (currentItem < 0 || currentItem >= static_cast<int>(materials.size())) currentItem = 1;
        if (ImGui::Combo(
                "Material",
                &currentItem,
                [](void* data, int idx, const char** out_text) {
                    auto* items = static_cast<std::vector<Application::DTO::Soils::SoilOptionDTO>*>(data);
                    if (idx < 0 || idx >= static_cast<int>(items->size())) return false;
                    *out_text = (*items)[idx].label.c_str();
                    return true;
                },
                (void*)&materials,
                static_cast<int>(materials.size()))) {
            params_.parentMaterial = materials[currentItem].code;
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
            Application::Services::SoilAnalysisService::applySoilSimulation(params_, visualizationLevel_, filter_);
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
        
        auto toggleCode = [](std::vector<int>& list, int code, bool isSelected) {
            if (isSelected) {
                auto it = std::remove(list.begin(), list.end(), code);
                list.erase(it, list.end());
            } else {
                list.push_back(code);
            }
        };

        // Filter 1: Orders
        if (ImGui::BeginCombo("Orders", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getOrders();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedOrders.begin(), filter_.allowedOrders.end(), option.code) != filter_.allowedOrders.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedOrders, option.code, isSelected);
                }
            }
            ImGui::EndCombo();
        }

        // Filter 2: Suborders
        if (ImGui::BeginCombo("Suborders", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getSuborders();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedSuborders.begin(), filter_.allowedSuborders.end(), option.code) != filter_.allowedSuborders.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedSuborders, option.code, isSelected);
                }
            }
            ImGui::EndCombo();
        }

        // Filter 3: Great Groups
        if (ImGui::BeginCombo("Gr. Groups", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getGreatGroups();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedGreatGroups.begin(), filter_.allowedGreatGroups.end(), option.code) != filter_.allowedGreatGroups.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedGreatGroups, option.code, isSelected);
                }
            }
            ImGui::EndCombo();
        }

        // Filter 4: Subgroups
        if (ImGui::BeginCombo("Subgroups", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getSubgroups();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedSubgroups.begin(), filter_.allowedSubgroups.end(), option.code) != filter_.allowedSubgroups.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedSubgroups, option.code, isSelected);
                }
            }
            ImGui::EndCombo();
        }

        // Filter 5: Families
        if (ImGui::BeginCombo("Families", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getFamilies();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedFamilies.begin(), filter_.allowedFamilies.end(), option.code) != filter_.allowedFamilies.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedFamilies, option.code, isSelected);
                }
            }
            ImGui::EndCombo();
        }

        // Filter 6: Series
        if (ImGui::BeginCombo("Series", "Select Allowed...")) {
            const auto options = Application::Services::SoilAnalysisService::getSeries();
            for (const auto& option : options) {
                bool isSelected = std::find(filter_.allowedSeries.begin(), filter_.allowedSeries.end(), option.code) != filter_.allowedSeries.end();
                if (ImGui::Selectable(option.label.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups)) {
                    toggleCode(filter_.allowedSeries, option.code, isSelected);
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
        ImGui::SameLine();
        if (ImGui::Button("Clear Classification")) {
            Application::Services::SoilAnalysisService::clearLastDetectedClasses();
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Legend (Colors)");
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Legend (Colors)", ImGuiTreeNodeFlags_DefaultOpen)) {
            const auto legendItems = Application::Services::SoilAnalysisService::getDetectedLegendItems(visualizationLevel_);

            if (legendItems.empty()) {
                ImGui::TextDisabled("(No soils detected matching filters)");
            }

            for (const auto& soil : legendItems) {
                ImGui::ColorButton("##legend", ImVec4(soil.r, soil.g, soil.b, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(16, 16));
                ImGui::SameLine();
                ImGui::Text("%s", soil.label.c_str());
            }
        }
        
    }

    ImGui::Separator();
    ImGui::Text("Analysis");
    ImGui::InputFloat("Raster Grid Size (m)", &rasterCellSize_, 0.5f, 5.0f, "%.1f");
    if (rasterCellSize_ < 0.1f) rasterCellSize_ = 0.1f;

    if (ImGui::Button("Rasterize & Analyze Patches", ImVec2(-1, 40))) {
        rasterStatus_ = "Starting Rasterization...";

        if (!patchAnalysisPanel_) {
            rasterStatus_ = "Error: Patch Panel not linked.";
        } else {
            auto terrainVertices = Application::Services::World3DService::getTerrainVertices();
            if (terrainVertices.empty()) {
                rasterStatus_ = "Error: No Active Terrain.";
            } else {
                // 1. Reconstruct Soil Classifications
                // Explicitly construct DTO to avoid type mismatch if params_ is Domain object
                Application::DTO::Soils::ScorpanParamsDTO paramsDTO;
                paramsDTO.rainfall = params_.rainfall;
                paramsDTO.temperature = params_.temperature;
                paramsDTO.vegetationDensity = params_.vegetationDensity;
                paramsDTO.ageFactor = params_.ageFactor;
                paramsDTO.parentMaterial = params_.parentMaterial;

                auto soilResult = Application::Services::SoilAnalysisService::classifyTerrain(terrainVertices, paramsDTO);

                // 2. Rasterize
                auto grid = Application::Services::SoilAnalysisService::rasterize(
                    terrainVertices,
                    soilResult.classes,
                    static_cast<double>(rasterCellSize_)
                );
                
                // 4. Save to Disk (for PatchAnalysisPanel to load)
                std::string csvPath = "assets/data/soil_raster_" + std::to_string((int)rasterCellSize_) + "m.csv";
                
                // Write Grid CSV manually here or use a helper? 
                // PatchAnalysis expects CSV. SoilRasterizer returning GridData is nice, but we need to serialize it.
                // Helper SoilRasterizer::SaveGridCsv? Or direct write?
                // Let's do direct write to keep it simple or check if PaatchAnalysis has a saver.
                // PatchAnalysis::WriteCsv writes METRICS, not the grid itself.
                // We need to write the GRID values.
                
                std::ofstream file(csvPath);
                if (file.is_open()) {
                    // Write Metadata Header
                    file << "# Origin: " << grid.originX << ", " << grid.originY << "\n";
                    
                    for (int y = 0; y < grid.height; y++) {
                        for (int x = 0; x < grid.width; x++) {
                            file << grid.values[y * grid.width + x];
                            if (x < grid.width - 1) file << ",";
                        }
                        file << "\n";
                    }
                    file.close();

                    // 4b. Save Elevation CSV (for 3D Overlay)
                    if (!grid.elevation.empty()) {
                         std::string elevPath = "assets/data/soil_elevation_" + std::to_string((int)rasterCellSize_) + "m.csv";
                         std::ofstream ef(elevPath);
                         if (ef.is_open()) {
                             for (int y = 0; y < grid.height; y++) {
                                for (int x = 0; x < grid.width; x++) {
                                    ef << grid.elevation[y * grid.width + x];
                                    if (x < grid.width - 1) ef << ",";
                                }
                                ef << "\n";
                             }
                         }
                    }

                    // 5. Save Legend
                    std::string legendPath = "assets/data/soil_legend.csv"; 
                    Application::Services::SoilAnalysisService::saveLegendCsv(legendPath, soilResult.uniqueClasses);

                    // 6. Trigger Panel
                    patchAnalysisPanel_->SetInputPath(csvPath);
                    patchAnalysisPanel_->SetLegendPath(legendPath);

                    rasterStatus_ = "Success! Grid saved to " + csvPath;
                } else {
                    rasterStatus_ = "Error: Could not write CSV.";
                }
            }
        }
    }
    if (!rasterStatus_.empty()) {
        ImGui::TextColored(ImVec4(1,1,0,1), "%s", rasterStatus_.c_str());
    }

    ImGui::End();
}

} // namespace UI::Panels
