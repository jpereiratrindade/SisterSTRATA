#include "WelcomePanel.hpp"
#include "imgui.h"

namespace UI::Panels {

void WelcomePanel::draw(bool* open, const Application::DTO::UIData& data) {
    if (!open || !(*open)) return;

    ImGuiIO& io = ImGui::GetIO();
    // Position on top-right, similar to Inspector in reference
    const float panelMargin = 10.0f;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320.0f - panelMargin, panelMargin + 18.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 200.0f), ImGuiCond_FirstUseEver); // Allow resize
    
    if (ImGui::Begin("Welcome to SisterSTRATA", open)) {
        std::string backend = (data.startMessage.find("CPU") != std::string::npos) ? "CPU BACKEND" : "VULKAN BACKEND";
        if (data.startMessage.find("View:") != std::string::npos) {
            backend += " " + data.startMessage.substr(data.startMessage.find("View:"));
        }
        ImGui::Text("Scientific Data Platform - %s", backend.c_str());
        ImGui::Separator();
        
        // Use DTO Data
        ImGui::Text("FPS: %.1f", data.framerate);
        ImGui::Text("Frame Time: %.3f ms", data.frameTimeMs);
        
        if (!data.startMessage.empty()) {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", data.startMessage.c_str());
        }

        if (ImGui::Button("Reset Camera")) {
                // TODO: Reset Camera logic
        }

        ImGui::Separator();
    
        ImGui::Separator();
        // Lighting controls moved to Settings
        ImGui::TextDisabled("Lighting controls moved to Tools > Settings");

    }
    ImGui::End();
}

} // namespace UI::Panels
