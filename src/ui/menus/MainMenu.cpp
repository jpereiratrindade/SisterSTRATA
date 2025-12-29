#include "MainMenu.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp" // For direct actions like ApplySlopeAnalysis if needed, or better, delegate? 
#include <filesystem>
// Original code called World3D::applySlopeAnalysis direct from menu. keeping specific logic here.

namespace UI::Menus {

void MainMenu::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...", "Ctrl+O")) {
                showOpenDialog = true;
                openFileSelector.Open(lastOpenPath);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                 if (World3D::getCurrentFilePath().empty()) {
                     showSaveAsDialog = true;
                     saveFileSelector.Open(lastSavePath.empty() ? "assets/data/" : lastSavePath);
                 } else {
                     if (onSaveFile) onSaveFile(World3D::getCurrentFilePath());
                 }
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                if (showSaveAsDialog) { // If already flagged
                } else {
                     std::string current = World3D::getCurrentFilePath();
                     showSaveAsDialog = true;
                     saveFileSelector.Open(current.empty() ? "assets/data/" : current);
                }
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
            if (ImGui::MenuItem("Analyze Drainage")) {
                World3D::DrainageStats stats = World3D::applyDrainageSimulation();
                drainageResultMsg = stats.message.empty() ? "Success" : stats.message;
                
                // Format detailed report
                if (stats.message.empty()) {
                    char buf[512];
                    snprintf(buf, sizeof(buf), 
                        "Drainage Analysis Complete.\n\n"
                        "Statistics:\n"
                        "- Max Accumulation: %d cells\n"
                        "- Mean Accumulation: %.2f cells\n"
                        "- River Cells (>50): %d\n\n"
                        "Visualization:\n"
                        "- Blue: Flow Accumulation\n"
                        "- Gradient: Elevation Depth", 
                        stats.maxAccumulation, stats.meanAccumulation, stats.riverCells);
                    drainageResultMsg = std::string(buf);
                }
                openDrainagePopup = true;
            }

            if (ImGui::MenuItem("Patch Analysis (CSV)")) {
                showPatchAnalysis = true;
            }
            if (ImGui::MenuItem("Generate Pattern")) {
                showTerrainGen = true;
            }
            if (ImGui::BeginMenu("Soil Simulation")) {
                if (ImGui::MenuItem("SCORPAN Parameters (Input)")) {
                    showScorpanWindow = true;
                }
                if (ImGui::MenuItem("SiBCS Viewer (Output)")) {
                    showSiBCSWindow = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }

    // Drainage Popup - Moved outside menu scope to ensure it renders
    if (openDrainagePopup) {
        ImGui::OpenPopup("Drainage Result");
        openDrainagePopup = false;
    }

    if (ImGui::BeginPopupModal("Drainage Result", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", drainageResultMsg.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    drawOpenFileDialog();
    drawSaveFileDialog();
}

void MainMenu::drawOpenFileDialog() {
    if (showOpenDialog) {
        // Init if needed (e.g. set path once)
        // fileSelector handles internal state.
        
        std::string result;
        if (openFileSelector.draw(&showOpenDialog, result, "")) { // Empty filter = all files, or .obj/.csv?
             if (onOpenFile) onOpenFile(result);
             std::filesystem::path selectedPath(result);
             if (selectedPath.has_parent_path()) {
                 lastOpenPath = selectedPath.parent_path().string();
             }
             showOpenDialog = false;
        }
    }
}

void MainMenu::drawSaveFileDialog() {
    if (showSaveAsDialog) {
        std::string result;
        if (saveFileSelector.draw(&showSaveAsDialog, result, "", true)) {
             if (onSaveFile) onSaveFile(result);
             std::filesystem::path selectedPath(result);
             if (selectedPath.has_parent_path()) {
                 lastSavePath = selectedPath.parent_path().string();
             }
             showSaveAsDialog = false;
        }
    }
}

} // namespace UI::Menus
