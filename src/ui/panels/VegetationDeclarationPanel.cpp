#include "VegetationDeclarationPanel.hpp"
#include "imgui.h"
#include "application/services/World3DService.hpp"
#include <cmath>

namespace UI::Panels {

VegetationDeclarationPanel::VegetationDeclarationPanel() {
    // defaults
}

void VegetationDeclarationPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    if (ImGui::Begin("Vegetation Original Declaration", open)) {
        ImGui::TextWrapped("System: VegetationSystemOriginal (Declarative)");
        ImGui::Separator();

        ImGui::InputText("Hypothesis ID", idBuffer_, sizeof(idBuffer_));
        
        const char* types[] = { "Campestre", "FlorestalNatural", "Agua" };
        ImGui::Combo("Vegetation Type", &selectedType_, types, IM_ARRAYSIZE(types));

        ImGui::Text("Relief Conditions (Plausibility)");
        ImGui::DragFloat("Min Slope (deg)", &minSlope_, 0.5f, 0.0f, 90.0f);
        ImGui::DragFloat("Max Slope (deg)", &maxSlope_, 0.5f, 0.0f, 90.0f);
        ImGui::DragFloat("Max Dist to Drainage (m)", &maxDistDrainage_, 1.0f, 0.0f, 5000.0f);

        ImGui::Separator();

        if (ImGui::Button("Declare Hypothesis")) {
            Application::DTO::Vegetation::DeclarationDTO dto;
            dto.id = std::string(idBuffer_);
            dto.typeCode = selectedType_;
            dto.minSlope = minSlope_;
            dto.maxSlope = maxSlope_;
            dto.maxDistDrainage = maxDistDrainage_;

            service_.declareHypothesis(dto);
        }

        ImGui::Separator();
        ImGui::Text("Declared Hypotheses & Technical Basis Check:");
        
        // Technical Base: Current Terrain Vertices
        // Accessing this is cheap, iterating is expensive.
        ImGui::Separator();
        ImGui::Text("Legend (Visualization Colors):");
        
        ImGui::ColorButton("##LegFlorestal", ImVec4(0.0f, 0.4f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
        ImGui::SameLine();
        ImGui::Text("Florestal Natural (Dark Green)");

        ImGui::ColorButton("##LegCampestre", ImVec4(0.6f, 0.8f, 0.2f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
        ImGui::SameLine();
        ImGui::Text("Campestre (Light Green)");

        ImGui::ColorButton("##LegAgua", ImVec4(0.0f, 0.4f, 0.8f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
        ImGui::SameLine();
        ImGui::Text("Agua (Blue)");

        ImGui::ColorButton("##LegNone", ImVec4(0.5f, 0.4f, 0.3f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
        ImGui::SameLine();
        ImGui::Text("Not Covered (Soil/Brown)");

        ImGui::Separator();
        ImGui::Text("Declared Hypotheses & Technical Bases (Grouped by Scenario):");
        
        auto terrainVertices = Application::Services::World3DService::getTerrainVertices();
        const auto& scenarios = service_.getScenarioDTOs(); 
        
        for (size_t i = 0; i < scenarios.size(); ++i) {
            auto& scenario = scenarios[i];
            std::string sid = scenario.id;
            
            // Append index to ID for UI uniqueness to handle duplicate IDs
            std::string uiId = sid + "##Scene" + std::to_string(i);
            
            if (ImGui::TreeNodeEx(uiId.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                // Reordering Buttons for Scenarios
                if (i > 0) {
                    ImGui::SameLine();
                    if (ImGui::Button("Up")) {
                        service_.swapScenarios(i, i - 1);
                        scenarioOutdated_ = true;
                        ImGui::TreePop(); 
                        break;
                    }
                }
                if (i < scenarios.size() - 1) {
                    ImGui::SameLine();
                    if (ImGui::Button("Down")) {
                        service_.swapScenarios(i, i + 1);
                        scenarioOutdated_ = true;
                        ImGui::TreePop();
                        break;
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Resolve This Scenario (Vector)")) {
                    const auto& hydro = Application::Services::World3DService::getHydroGrid();
                    float spacing = 2.0f; 
                    if (terrainVertices.size() > 1) {
                        float d = std::abs(terrainVertices[1].pos.x - terrainVertices[0].pos.x);
                        if (d > 0.001f) spacing = d;
                    }

                    auto result = service_.resolveScenarioToCodes(i, terrainVertices, hydro, spacing);
                    scenarioOutdated_ = true; // Mark scenario (indices) as outdated since we switched mode
                    
                    service_.applyClassificationVisualization(result);
                }

                ImGui::SameLine();
                if (ImGui::Button("Suprimir Scenario")) {
                    service_.removeScenarioByIndex(i);
                    ImGui::TreePop();
                    break;
                }

                const auto& components = scenario.components;
                for (size_t j = 0; j < components.size(); ++j) {
                    const auto& h = components[j];
                    std::string compId = h.typeLabel + "##" + std::to_string(j);
                    
                    CachedStats& stats = statsCache_[sid + "_" + compId]; 

                    ImGui::PushID(compId.c_str());
                    ImGui::BulletText("%s", h.typeLabel.c_str());

                    ImGui::Indent();
                    ImGui::Text("Criteria: Slope %.1f-%.1f deg, Dist < %.0f m", 
                        h.minSlope.value_or(0),
                        h.maxSlope.value_or(90),
                        h.maxDistanceToDrainage.value_or(9999)
                    );

                    if (ImGui::Button("Verify Coverage")) {
                        const auto& hydro = Application::Services::World3DService::getHydroGrid();
                        float spacing = 2.0f; 
                        if (terrainVertices.size() > 1) {
                            float d = std::abs(terrainVertices[1].pos.x - terrainVertices[0].pos.x);
                            if (d > 0.001f) spacing = d;
                        }
                        
                        auto result = service_.calculatePotentialCoverage(i, j, terrainVertices, hydro, spacing);
                        stats.matchVertices = result.matchVertices;
                        stats.coveragePercentage = result.coveragePercentage;
                        stats.outdated = false;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Visualize (Apply)")) {
                        const auto& hydro = Application::Services::World3DService::getHydroGrid();
                        float spacing = 2.0f; 
                        if (terrainVertices.size() > 1) {
                            float d = std::abs(terrainVertices[1].pos.x - terrainVertices[0].pos.x);
                            if (d > 0.001f) spacing = d;
                        }

                        auto result = service_.calculatePotentialCoverage(i, j, terrainVertices, hydro, spacing);
                        
                        stats.matchVertices = result.matchVertices;
                        stats.coveragePercentage = result.coveragePercentage;
                        stats.outdated = false;

                        service_.applyVegetationVisualization(i, j, result.coverageMask);
                    }

                    if (!stats.outdated) {
                         ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
                            "Potential Coverage: %.1f%% (%zu vertices)", 
                            stats.coveragePercentage, stats.matchVertices);
                    } else {
                        ImGui::TextDisabled("Coverage not verified.");
                    }

                    ImGui::Unindent();
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }

        ImGui::Separator();
        ImGui::Text("Global Hypothesis Resolution (Overlay Mode)");
        ImGui::TextDisabled("Priority: Top of list wins (Scenarios exclude each other)");
        
        if (ImGui::Button("Resolve All scenarios (Overlap)")) {
             const auto& hydro = Application::Services::World3DService::getHydroGrid();
             float spacing = 2.0f; 
             if (terrainVertices.size() > 1) {
                 float d = std::abs(terrainVertices[1].pos.x - terrainVertices[0].pos.x);
                 if (d > 0.001f) spacing = d;
             }
             
             service_.calculateScenario(terrainVertices, hydro, spacing);
             scenarioOutdated_ = false;
        }
        
        if (!scenarioOutdated_) {
            ImGui::SameLine();
            if (ImGui::Button("Visualize Global Resolution")) {
                const auto* res = service_.getLastScenarioResult();
                if (res && !res->semanticCodes.empty()) {
                    Application::Services::World3DService::applyClassificationVisualization(res->semanticCodes);
                }
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
