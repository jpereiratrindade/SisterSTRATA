#pragma once
#include <functional>
#include <string>

namespace UI::Menus {

class MainMenu {
public:
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
    bool showTerrainGen = false;
    bool showWelcome = true; // Default true as per original
    bool showSoilSim = false; // New

private:
    bool showOpenDialog = false;
    bool showSaveAsDialog = false; // New
    char filePathBuf[256] = "assets/data/sample.csv";
    char saveFilePathBuf[256] = "assets/data/meshexport.obj"; // New

    void drawOpenFileDialog();
    void drawSaveFileDialog(); // New
};

} // namespace UI::Menus
