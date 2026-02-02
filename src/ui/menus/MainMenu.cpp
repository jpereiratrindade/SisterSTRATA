#include "MainMenu.hpp"
#include "imgui.h"
#include "application/services/World3DService.hpp" // For direct actions like ApplySlopeAnalysis if needed, or better, delegate? 
#include <filesystem>
// Original code called Application::Services::World3DService::applySlopeAnalysis direct from menu. keeping specific logic here.

namespace UI::Menus {

void MainMenu::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...", "Ctrl+O")) {
                showOpenDialog = true;
                openFileSelector.Open(lastOpenPath);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                 if (Application::Services::World3DService::getCurrentFilePath().empty()) {
                     showSaveAsDialog = true;
                     saveFileSelector.Open(lastSavePath.empty() ? "assets/data/" : lastSavePath);
                 } else {
                     if (onSaveFile) onSaveFile(Application::Services::World3DService::getCurrentFilePath());
                 }
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                if (showSaveAsDialog) { // If already flagged
                } else {
                     std::string current = Application::Services::World3DService::getCurrentFilePath();
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
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Visualization (Original Colors)")) {
                 Application::Services::World3DService::resetVisualization();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Settings")) {
                showSettings = true;
            }
            if (ImGui::MenuItem("Analyze Slope")) {
                Application::Services::World3DService::applySlopeAnalysis(); // Call logic directly as before
                showAnalysisReport = true;
            }
            if (ImGui::MenuItem("Hydrology Inspector")) {
                showHydrologyPanel = true;
            }
            if (ImGui::MenuItem("Vegetation Declaration (Original)")) {
                showVegetation = true;
            }
            if (ImGui::MenuItem("Fourth Dimension (Resilience)")) {
                showTimeline = true;
            }
            if (ImGui::MenuItem("Terrain Simulator (Procedural)")) {
                showTerrainGenerator = true;
            }
            if (ImGui::MenuItem("Narrative Observation Context")) {
                showNarrativePanel = true;
            }
            if (ImGui::MenuItem("Discursive System Context")) {
                showDiscursivePanel = true;
            }
            if (ImGui::MenuItem("Recommendation Trajectory Context")) {
                showRecommendationPanel = true;
            }

            if (ImGui::MenuItem("Patch Analysis (CSV)")) {
                showPatchAnalysis = true;
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
