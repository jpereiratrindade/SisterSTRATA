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
        ImGui::Text("Declared Hypotheses & Technical Bases:");
        
        // Technical Base: Current Terrain Vertices
        // Accessing this is cheap, iterating is expensive.
        const auto& vertices = World3D::getVertices();


        
        // Use index-based loop to support granular deletion
        auto& hypotheses = system_.getHypotheses(); 
        // Note: getHypotheses returns const ref. We modify system via method.
        
        for (size_t i = 0; i < hypotheses.size(); ++i) {
            const auto& h = hypotheses[i];
            std::string hid = h.getId().getValue();
            
            // Append index to ID for UI uniqueness to handle duplicate IDs
            std::string uiId = hid + "##" + std::to_string(i);
            
            CachedStats& stats = statsCache_[uiId]; 

            ImGui::PushID(uiId.c_str());
            ImGui::BulletText("%s: %s", hid.c_str(), h.getType().toString().c_str());

            // Reordering Buttons
            if (i > 0) {
                ImGui::SameLine();
                if (ImGui::Button("Up")) {
                    system_.swapHypotheses(i, i - 1);
                    scenarioOutdated_ = true;
                    ImGui::PopID(); // Clean up current ID stack
                    break;          // Break loop to refresh
                }
            }
            if (i < hypotheses.size() - 1) {
                ImGui::SameLine();
                if (ImGui::Button("Down")) {
                    system_.swapHypotheses(i, i + 1);
                    scenarioOutdated_ = true;
                    ImGui::PopID();
                    break;
                }
            }
            
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
                 
                 // Check dependencies
                 if (h.getConditions().maxDistanceToDrainage.has_value() && (!hydro.isValid() || hydro.flowAccumulationCells.empty())) {
                     // Warning will be shown in the UI below, but we run anyway (ignoring drainage)
                 }

                 auto result = Core::Domain::Vegetation::VegetationMappingService::calculatePotentialCoverage(
                     h, vertices, hydro, spacing
                 );
                 stats.matchVertices = result.matchVertices;
                 stats.coveragePercentage = result.coveragePercentage;
                 stats.outdated = false;
            }

            // Warning for missing dependencies
            const auto& hydroGrid = World3D::getHydroGrid();
            if (h.getConditions().maxDistanceToDrainage.has_value() && (!hydroGrid.isValid() || hydroGrid.flowAccumulationCells.empty())) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[!] Run 'Compute Drainage' first!");
            }
            ImGui::SameLine();
            if (ImGui::Button("Visualize (Apply)")) {
                // Determine dependencies
                 const auto& hydro = World3D::getHydroGrid();
                 float spacing = 2.0f; 
                 if (vertices.size() > 1) {
                     float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                     if (d > 0.001f) spacing = d;
                 }

                 auto result = Core::Domain::Vegetation::VegetationMappingService::calculatePotentialCoverage(
                     h, vertices, hydro, spacing
                 );
                 
                 // Update stats too
                 stats.matchVertices = result.matchVertices;
                 stats.coveragePercentage = result.coveragePercentage;
                 stats.outdated = false;

                World3D::applyVegetationVisualization(h, result.coverageMask);
            }
            ImGui::SameLine();
            if (ImGui::Button("Suprimir")) {
                system_.removeHypothesisByIndex(i);
                ImGui::Unindent();
                ImGui::PopID();
                break; 
            }

            if (!stats.outdated) {
                if (stats.realizedPercentage >= 0.0f) {
                     ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
                        "Coverage: Potential %.1f%% | Realized %.1f%% (Exclusive)", 
                        stats.coveragePercentage, stats.realizedPercentage);
                } else {
                     ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
                        "Potential Coverage: %.1f%% (%zu vertices)", 
                        stats.coveragePercentage, stats.matchVertices);
                }
            } else {
                ImGui::TextDisabled("Coverage not verified.");
            }
            ImGui::Unindent();
            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::Text("Global Hypothesis Resolution");
        ImGui::TextDisabled("Priority: Top of list wins (Types exclude each other)");
        
        if (ImGui::Button("Resolve Scenario (Overlay)")) {
             const auto& hydro = World3D::getHydroGrid();
             const auto& vertices = World3D::getVertices();
             float spacing = 2.0f; 
             if (vertices.size() > 1) {
                 float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
                 if (d > 0.001f) spacing = d;
             }
             
             auto& hypotheses = system_.getHypotheses();
             lastScenario_ = Core::Domain::Vegetation::VegetationMappingService::calculateScenario(
                 hypotheses, vertices, hydro, spacing
             );
             scenarioOutdated_ = false;
             
             // Update stats
             for (size_t i = 0; i < hypotheses.size(); ++i) {
                 std::string hid = hypotheses[i].getId().getValue();
                 std::string uiId = hid + "##" + std::to_string(i);
                 statsCache_[uiId].realizedPercentage = lastScenario_.stats[i].realizedPercentage;
             }
        }
        
        if (!scenarioOutdated_) {
            ImGui::SameLine();
            if (ImGui::Button("Visualize Resolution (All)")) {
                 const auto& vertices = World3D::getVertices();
                 const auto& hypotheses = system_.getHypotheses();
                 
                 for (size_t i = 0; i < hypotheses.size(); ++i) {
                     std::vector<bool> mask(vertices.size(), false);
                     const auto& cls = lastScenario_.classification;
                     if (cls.size() != vertices.size()) continue;

                     for(size_t v=0; v<vertices.size(); ++v) {
                         if (cls[v] == (int)i) mask[v] = true;
                     }
                     // First layer should NOT accumulate (clears background)
                     // Subsequent layers SHOULD accumulate
                     bool accum = (i > 0);
                     World3D::applyVegetationVisualization(hypotheses[i], mask, accum); 
                 }
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
