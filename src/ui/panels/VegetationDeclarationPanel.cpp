#include "VegetationDeclarationPanel.hpp"
#include "imgui.h"
#include "core/domain/vegetation/VegetationMappingService.hpp"
#include "world3d/World3D.hpp"

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
            Core::Domain::Vegetation::DTOs::VegetationDeclarationDTO dto;
            dto.id = std::string(idBuffer_);
            dto.typeCode = static_cast<Core::Domain::Vegetation::VegetationCode>(selectedType_);
            dto.minSlope = minSlope_;
            dto.maxSlope = maxSlope_;
            dto.maxDistDrainage = maxDistDrainage_;

            auto hypo = service_.createHypothesis(dto);
            system_.addHypothesis(hypo);
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
        
        const auto& vertices = World3D::getVertices();
        auto& scenarios = system_.getScenarios(); 
        
        for (size_t i = 0; i < scenarios.size(); ++i) {
            auto& scenario = scenarios[i];
            std::string sid = scenario.getId();
            
            // Append index to ID for UI uniqueness to handle duplicate IDs
            std::string uiId = sid + "##Scene" + std::to_string(i);
            
            if (ImGui::TreeNodeEx(uiId.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                // Reordering Buttons for Scenarios
                if (i > 0) {
                    ImGui::SameLine();
                    if (ImGui::Button("Up")) {
                        system_.swapScenarios(i, i - 1);
                        scenarioOutdated_ = true;
                        ImGui::TreePop(); 
                        break;
                    }
                }
                if (i < scenarios.size() - 1) {
                    ImGui::SameLine();
                    if (ImGui::Button("Down")) {
                        system_.swapScenarios(i, i + 1);
                        scenarioOutdated_ = true;
                        ImGui::TreePop();
                        break;
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Resolve This Scenario (Vector)")) {
                    const auto& hydro = World3D::getHydroGrid();
                    float spacing = 2.0f; 
                    if (vertices.size() > 1) {
                        float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                        if (d > 0.001f) spacing = d;
                    }

                    lastSemanticClassification_ = Core::Domain::Vegetation::VegetationMappingService::resolveScenarioToCodes(
                        scenario, vertices, hydro, spacing
                    );
                    semanticActive_ = true;
                    scenarioOutdated_ = true; // Mark scenario (indices) as outdated since we switched mode
                    
                    World3D::applyClassificationVisualization(lastSemanticClassification_);
                }

                ImGui::SameLine();
                if (ImGui::Button("Suprimir Scenario")) {
                    system_.removeScenarioByIndex(i);
                    ImGui::TreePop();
                    break;
                }

                const auto& components = scenario.getComponents();
                for (size_t j = 0; j < components.size(); ++j) {
                    const auto& h = components[j];
                    std::string compId = h.getType().toString() + "##" + std::to_string(j);
                    
                    CachedStats& stats = statsCache_[sid + "_" + compId]; 

                    ImGui::PushID(compId.c_str());
                    ImGui::BulletText("%s", h.getType().toString().c_str());

                    ImGui::Indent();
                    ImGui::Text("Criteria: Slope %.1f-%.1f deg, Dist < %.0f m", 
                        h.getConditions().minSlope.value_or(0),
                        h.getConditions().maxSlope.value_or(90),
                        h.getConditions().maxDistanceToDrainage.value_or(9999)
                    );

                    if (ImGui::Button("Verify Coverage")) {
                        const auto& hydro = World3D::getHydroGrid();
                        float spacing = 2.0f; 
                        if (vertices.size() > 1) {
                            float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                            if (d > 0.001f) spacing = d;
                        }
                        
                        auto result = Core::Domain::Vegetation::VegetationMappingService::calculatePotentialCoverage(
                            h, vertices, hydro, spacing
                        );
                        stats.matchVertices = result.matchVertices;
                        stats.coveragePercentage = result.coveragePercentage;
                        stats.outdated = false;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Visualize (Apply)")) {
                        const auto& hydro = World3D::getHydroGrid();
                        float spacing = 2.0f; 
                        if (vertices.size() > 1) {
                            float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                            if (d > 0.001f) spacing = d;
                        }

                        auto result = Core::Domain::Vegetation::VegetationMappingService::calculatePotentialCoverage(
                            h, vertices, hydro, spacing
                        );
                        
                        stats.matchVertices = result.matchVertices;
                        stats.coveragePercentage = result.coveragePercentage;
                        stats.outdated = false;

                        World3D::applyVegetationVisualization(h, result.coverageMask);
                        semanticActive_ = false; // Visualization is now mixed/partial
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
             const auto& hydro = World3D::getHydroGrid();
             const auto& vertices = World3D::getVertices();
             float spacing = 2.0f; 
             if (vertices.size() > 1) {
                 float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                 if (d > 0.001f) spacing = d;
             }
             
             auto& scenarios = system_.getScenarios();
             lastScenario_ = Core::Domain::Vegetation::VegetationMappingService::calculateScenario(
                 scenarios, vertices, hydro, spacing
             );
             scenarioOutdated_ = false;
             semanticActive_ = false; 
        }
        
        if (!scenarioOutdated_) {
            ImGui::SameLine();
            if (ImGui::Button("Visualize Global Resolution")) {
                 if (!lastScenario_.semanticCodes.empty()) {
                     World3D::applyClassificationVisualization(lastScenario_.semanticCodes);
                 }
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
