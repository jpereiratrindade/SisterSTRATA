#pragma once
#include <functional>
#include <string>
#include "ui/components/FileSelector.hpp"
#include "application/services/World3DService.hpp" // For DrainageStats

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
    std::function<void(std::string)> onOpenFile; // Path
    std::function<void(std::string)> onSaveFile; // Path
    std::function<void(std::string)> onOpenProject; // Path
    std::function<void(std::string)> onNewProject; // Path
    std::function<void()> onCloseFile;
    std::function<void()> onExit;

    // State control for other panels
    bool showSettings = false;
    bool showAnalysisReport = false;
    bool showPatchAnalysis = false;
    bool showHydrologyPanel = false;
    bool showTerrainGenerator = false; // New
    bool showVegetation = false; // New
    bool showWelcome = true; // Default true as per original
    bool showScorpanWindow = false; // New
    bool showSiBCSWindow = false;   // New
    bool showTimeline = false;      // New (FourthDimension)
    bool showNarrativePanel = false; // New (Observational)
    bool showDiscursivePanel = false;
    bool showRecommendationPanel = false;
    bool* showGlobalSynthesis = nullptr; // New: Pointer to UserInterface state
    std::string currentProjectPath; // New: Cached path for display
private:
    void drawOpenFileDialog();
    void drawSaveFileDialog(); // New
    void drawProjectDialog();

    bool showOpenDialog = false;
    bool showSaveAsDialog = false;
    bool showProjectDialog = false;
    bool isNewProjectMode = false;
    bool projectDialogOpenRequested = false;
    
    UI::Components::FileSelector openFileSelector{"Open File"};
    UI::Components::FileSelector saveFileSelector{"Save As"};
    UI::Components::FileBrowser projectBrowser{"Project Browser"};
    
    std::string lastOpenPath = "assets/data/";
    std::string lastSavePath = "assets/data/meshexport.obj";
    std::string lastProjectPath = "assets/data/user_db";
};

} // namespace UI::Menus
