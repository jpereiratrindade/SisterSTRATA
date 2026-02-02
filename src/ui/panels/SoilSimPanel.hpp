#pragma once
#include "imgui.h"
#include "application/dtos/SoilDTOs.hpp"
#include <string>

namespace UI::Panels {

class PatchAnalysisPanel; // Forward declaration

class SoilSimPanel {
public:
    void drawScorpan(bool* open);
    void drawSiBCS(bool* open);
    
    void setPatchAnalysisPanel(PatchAnalysisPanel* panel) { patchAnalysisPanel_ = panel; }

private:
    
private:
    Application::DTO::Soils::ScorpanParamsDTO params_;
    int visualizationLevel_ = 1; // 1=Order, 2=Suborder, 3=GreatGroup
    Application::DTO::Soils::SiBCSFilterDTO filter_; // New
    PatchAnalysisPanel* patchAnalysisPanel_ = nullptr;
    float rasterCellSize_ = 1.0f;
    std::string rasterStatus_;
};

} // namespace UI::Panels
