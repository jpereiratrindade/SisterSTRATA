#include "SoilSimPanel.hpp"
#include "world3d/World3D.hpp"
#include "core/domain/soils/SoilSystem.hpp" // Added
#include "core/domain/soils/SiBCS.hpp" // Added
#include "core/domain/spatial_pattern/SoilRasterizer.hpp" 
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
                params_.parentMaterial = Core::Domain::Soils::ParentMaterialType::Sedimentary;
            } else if (currentPreset == 2) { // Young
                params_.rainfall = 1800.0f;
                params_.temperature = 22.0f;
                params_.ageFactor = 0.2f; // Only young soils (Cambissolos, Neossolos)
                params_.vegetationDensity = 0.4f;
                params_.parentMaterial = Core::Domain::Soils::ParentMaterialType::Igneous;
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
        ImGui::SameLine();
        if (ImGui::Button("Clear Classification")) {
            Core::Domain::Soils::SoilSystem::clearLastDetectedClasses();
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Legend (Colors)");
        ImGui::Spacing();

        // Universal Legend Logic 
        auto commonSoils = Core::Domain::Soils::SiBCSHelper::getCommonVectors(visualizationLevel_);
        if (ImGui::CollapsingHeader("Legend (Colors)", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Use ACTUAL detected classes from the system
            const auto& legendItems = Core::Domain::Soils::SoilSystem::getLastDetectedClasses();

            if (legendItems.empty()) {
               ImGui::TextDisabled("(No soils detected matching filters)");
            }

            for (const auto& soil : legendItems) {
                glm::vec3 color = Core::Domain::Soils::SiBCSHelper::getColor(soil, visualizationLevel_);
                ImGui::ColorButton("##legend", ImVec4(color.r, color.g, color.b, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(16, 16));
                ImGui::SameLine();
                ImGui::Text("%s", Core::Domain::Soils::SiBCSHelper::getName(soil, visualizationLevel_).c_str());
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
            const auto& vertices = World3D::getVertices();
            if (vertices.empty()) {
                rasterStatus_ = "Error: No Active Terrain.";
            } else {
                // 1. Calculate Bounds for Relative Elevation (needed for Prediction)
                float minZ = 1e9f;
                float maxZ = -1e9f;
                for (const auto& v : vertices) {
                    minZ = std::min(minZ, v.pos.z);
                    maxZ = std::max(maxZ, v.pos.z);
                }
                if (maxZ == minZ) maxZ = minZ + 1.0f;

                // 2. Reconstruct Soil Classifications
                std::vector<Core::Domain::Soils::SiBCSClassification> classes;
                classes.reserve(vertices.size());
                
                std::vector<Core::Domain::Soils::SiBCSClassification> uniqueClasses;

                for (const auto& v : vertices) {
                    float dot = std::clamp(v.normal.z, -1.0f, 1.0f);
                    float slopeDeg = glm::degrees(std::acos(dot));
                    float relElev = (v.pos.z - minZ) / (maxZ - minZ);
                    
                    // Call Public Predict
                    auto c = Core::Domain::Soils::SoilSystem::predict(params_, slopeDeg, v.pos.z, relElev);
                    classes.push_back(c);

                    // Track uniques for legend
                    bool exists = false;
                    for(const auto& u : uniqueClasses) if(u == c) { exists = true; break; }
                    if (!exists) uniqueClasses.push_back(c);
                }

                // 3. Rasterize
                auto grid = Core::Domain::SpatialPattern::SoilRasterizer::Rasterize(vertices, classes, (double)rasterCellSize_);
                
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
                    Core::Domain::SpatialPattern::SoilRasterizer::SaveLegendCsv(legendPath, uniqueClasses);

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
