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
    std::function<void()> onCloseFile;
    std::function<void()> onExit;

    // State control for other panels
    bool showSettings = false;
    bool showAnalysisReport = false;
    bool showTerrainGen = false;
    bool showWelcome = true; // Default true as per original

private:
    bool showOpenDialog = false;
    char filePathBuf[256] = "assets/data/sample.csv";

    void drawOpenFileDialog();
};

} // namespace UI::Menus
