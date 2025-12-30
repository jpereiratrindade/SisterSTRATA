#pragma once
#include <functional>
#include <string>
#include "ui/components/FileSelector.hpp"
#include "world3d/World3D.hpp" // For DrainageStats

namespace UI::Menus {

/**
 * @brief Main menu bar and top-level tool toggles.
 */
class MainMenu {
public:
    /**
     * @brief Render the main menu bar and dialogs.
     */
    void draw();

    // Callbacks to trigger Application/UI actions
    std::function<void()> onLoadDemo;
    std::function<void(std::string)> onOpenFile;
    std::function<void(std::string)> onSaveFile; // New
    std::function<void()> onCloseFile;
    std::function<void()> onExit;

    // State control for other panels
    bool showSettings = false;
    bool showAnalysisReport = false;
    bool showPatchAnalysis = false;
    bool showTerrainGen = false;
    bool showHydrologyPanel = false;
    bool showWelcome = true; // Default true as per original
    bool showScorpanWindow = false; // New
    bool showSiBCSWindow = false;   // New
    

private:
    bool showOpenDialog = false;
    bool showSaveAsDialog = false; 
    
    UI::Components::FileSelector openFileSelector{"Open File"};
    UI::Components::FileSelector saveFileSelector{"Save As"};
    
    char filePathBuf[256] = ""; // Legacy/Temp buffer if needed, or remove? Keeping for robust fallback if selector fails? No, simpler to use selector results.
    // Actually, FileSelector handles the path internally but returns string.
    
    // char filePathBuf... -> Removed in favor of direct string passing? 
    // Wait, MainMenu.cpp uses filePathBuf. I will refactor MainMenu.cpp to use local strings or the selector's return.
    // For now, I'll keep them as members if I want to pre-fill them, OR just rely on logic.
    // Let's remove them to clean up.
    std::string lastOpenPath = "assets/data/";
    std::string lastSavePath = "assets/data/meshexport.obj";

    void drawOpenFileDialog();
    void drawSaveFileDialog(); // New
};

} // namespace UI::Menus
