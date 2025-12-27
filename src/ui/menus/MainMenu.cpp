#include "MainMenu.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp" // For direct actions like ApplySlopeAnalysis if needed, or better, delegate? 
// Original code called World3D::applySlopeAnalysis direct from menu. keeping specific logic here.

namespace UI::Menus {

void MainMenu::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...", "Ctrl+O")) {
                showOpenDialog = true;
            }
            if (ImGui::MenuItem("Load Demo Cloud")) {
                if (onLoadDemo) onLoadDemo();
            }
            if (ImGui::MenuItem("Close File")) {
                if (onCloseFile) onCloseFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                if (onExit) onExit();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Welcome Panel", nullptr, &showWelcome);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Settings")) {
                showSettings = true;
            }
            if (ImGui::MenuItem("Analyze Slope")) {
                World3D::applySlopeAnalysis(); // Call logic directly as before
                showAnalysisReport = true;
            }
            if (ImGui::MenuItem("Generate Pattern")) {
                showTerrainGen = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }

    drawOpenFileDialog();
}

void MainMenu::drawOpenFileDialog() {
    // File Open Modal
    if (showOpenDialog) {
        ImGui::OpenPopup("Open File");
    }

    if (ImGui::BeginPopupModal("Open File", &showOpenDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter file path:");
        ImGui::InputText("Path", filePathBuf, IM_ARRAYSIZE(filePathBuf));
        
        ImGui::Separator();
        
        if (ImGui::Button("Load", ImVec2(120, 0))) {
            if (onOpenFile) onOpenFile(std::string(filePathBuf));
            showOpenDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            showOpenDialog = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

} // namespace UI::Menus
