#include "VegetationDeclarationPanel.hpp"
#include "imgui.h"

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
        
        const char* types[] = { "Campestre", "FlorestalNatural" };
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
        ImGui::Text("Declared Hypotheses:");
        for (const auto& h : system_.getHypotheses()) {
            ImGui::BulletText("%s: %s (Slope: %.1f-%.1f)", 
                h.getId().getValue().c_str(), 
                h.getType().toString().c_str(),
                h.getConditions().minSlope.value_or(0),
                h.getConditions().maxSlope.value_or(90)
            );
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
