#include "SettingsPanel.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp"
#include <glm/glm.hpp>

namespace UI::Panels {

void SettingsPanel::draw(bool* open) {
     if (!open || !(*open)) return;

    if (ImGui::Begin("Settings", open)) {
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 lightDir = World3D::getLightDirection();
                if (ImGui::SliderFloat3("Light Direction", &lightDir.x, -5.0f, 5.0f)) {
                    World3D::setLightDirection(lightDir.x, lightDir.y, lightDir.z);
                }

                glm::vec3 lightColor = World3D::getLightColor();
                if (ImGui::ColorEdit3("Light Color", &lightColor.r)) {
                    World3D::setLightColor(lightColor.r, lightColor.g, lightColor.b);
                }

                float ambient = World3D::getAmbientStrength();
                if (ImGui::SliderFloat("Ambient Strength", &ambient, 0.0f, 1.0f)) {
                    World3D::setAmbientStrength(ambient);
                }
        
            static float moveSpeed = 2.5f;
            if (ImGui::SliderFloat("Camera Speed", &moveSpeed, 0.1f, 100.0f)) {
                World3D::setCameraSpeed(moveSpeed);
            }
        }

        if (ImGui::CollapsingHeader("Basin Boundaries / Point Clouds")) {
            static float boundaryPointSize = 4.0f;
            if (ImGui::SliderFloat("Point Size", &boundaryPointSize, 1.0f, 20.0f, "%.1f")) {
                World3D::setPointSize(boundaryPointSize);
            }

            static int colorMode = 0;
            static glm::vec3 boundaryColor(1.0f, 1.0f, 1.0f);
            const char* modes[] = {"Source (CSV colors)", "Single color"};
            if (ImGui::Combo("Color Mode", &colorMode, modes, IM_ARRAYSIZE(modes))) {
                World3D::applyPointCloudColorMode(colorMode, boundaryColor);
            }
            if (colorMode == 1) {
                if (ImGui::ColorEdit3("Boundary Color", &boundaryColor.r)) {
                    World3D::applyPointCloudColorMode(colorMode, boundaryColor);
                }
            }
            if (ImGui::Button("Apply to Active Point Cloud")) {
                World3D::applyPointCloudColorMode(colorMode, boundaryColor);
            }
            ImGui::TextDisabled("Applies to the active point/line object.");
            ImGui::TextDisabled("Line width is fixed by the GPU.");
        }
        
        if (ImGui::CollapsingHeader("Performance")) {
            bool vsync = World3D::getVSync();
            if (ImGui::Checkbox("Enable VSync", &vsync)) {
                World3D::setVSync(vsync);
            }

            int targetFps = World3D::getTargetFPS();
            if (ImGui::SliderInt("Max FPS (0 = Uncapped)", &targetFps, 0, 240)) {
                World3D::setTargetFPS(targetFps);
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
