#include "SettingsPanel.hpp"
#include "imgui.h"
#include "application/services/World3DService.hpp"
#include <glm/glm.hpp>

namespace UI::Panels {

void SettingsPanel::setMultiViewportControls(bool* requested, const bool* supported, const bool* active) {
    multiViewportRequested_ = requested;
    multiViewportSupported_ = supported;
    multiViewportActive_ = active;
}

void SettingsPanel::draw(bool* open) {
     if (!open || !(*open)) return;

    if (ImGui::Begin("Settings", open)) {
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 lightDir = Application::Services::World3DService::getLightDirection();
                if (ImGui::SliderFloat3("Light Direction", &lightDir.x, -5.0f, 5.0f)) {
                    Application::Services::World3DService::setLightDirection(lightDir.x, lightDir.y, lightDir.z);
                }

                glm::vec3 lightColor = Application::Services::World3DService::getLightColor();
                if (ImGui::ColorEdit3("Light Color", &lightColor.r)) {
                    Application::Services::World3DService::setLightColor(lightColor.r, lightColor.g, lightColor.b);
                }

                float ambient = Application::Services::World3DService::getAmbientStrength();
                if (ImGui::SliderFloat("Ambient Strength", &ambient, 0.0f, 1.0f)) {
                    Application::Services::World3DService::setAmbientStrength(ambient);
                }
        
            static float moveSpeed = 2.5f;
            if (ImGui::SliderFloat("Camera Speed", &moveSpeed, 0.1f, 100.0f)) {
                Application::Services::World3DService::setCameraSpeed(moveSpeed);
            }
        }

        if (ImGui::CollapsingHeader("Basin Boundaries / Point Clouds")) {
            static float boundaryPointSize = 4.0f;
            if (ImGui::SliderFloat("Point Size", &boundaryPointSize, 1.0f, 20.0f, "%.1f")) {
                Application::Services::World3DService::setPointSize(boundaryPointSize);
            }

            static int colorMode = 0;
            static glm::vec3 boundaryColor(1.0f, 1.0f, 1.0f);
            const char* modes[] = {"Source (CSV colors)", "Single color"};
            if (ImGui::Combo("Color Mode", &colorMode, modes, IM_ARRAYSIZE(modes))) {
                Application::Services::World3DService::applyPointCloudColorMode(colorMode, boundaryColor);
            }
            if (colorMode == 1) {
                if (ImGui::ColorEdit3("Boundary Color", &boundaryColor.r)) {
                    Application::Services::World3DService::applyPointCloudColorMode(colorMode, boundaryColor);
                }
            }
            if (ImGui::Button("Apply to Active Point Cloud")) {
                Application::Services::World3DService::applyPointCloudColorMode(colorMode, boundaryColor);
            }
            ImGui::TextDisabled("Applies to the active point/line object.");
            ImGui::TextDisabled("Line width is fixed by the GPU.");
        }
        
        if (ImGui::CollapsingHeader("Performance")) {
            bool vsync = Application::Services::World3DService::getVSync();
            if (ImGui::Checkbox("Enable VSync", &vsync)) {
                Application::Services::World3DService::setVSync(vsync);
            }

            int targetFps = Application::Services::World3DService::getTargetFPS();
            if (ImGui::SliderInt("Max FPS (0 = Uncapped)", &targetFps, 0, 240)) {
                Application::Services::World3DService::setTargetFPS(targetFps);
            }
        }

        if (ImGui::CollapsingHeader("Workspace Windows", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool requested = multiViewportRequested_ ? *multiViewportRequested_ : false;
            const bool supported = multiViewportSupported_ ? *multiViewportSupported_ : false;
            const bool active = multiViewportActive_ ? *multiViewportActive_ : false;

            if (ImGui::Checkbox("Enable Multi-Viewport (separate OS windows)", &requested)) {
                if (requested && !supported) {
                    requested = false;
                }
                if (multiViewportRequested_) {
                    *multiViewportRequested_ = requested;
                }
            }

            ImGui::TextDisabled("Supported by current SDL backend: %s", supported ? "yes" : "no");
            ImGui::TextDisabled("Active now: %s", active ? "yes" : "no");
            if (!supported) {
                ImGui::TextWrapped("Current backend does not support platform viewports. Use X11 backend to enable separate OS windows.");
            }
        }
    }
    ImGui::End();
}

} // namespace UI::Panels
