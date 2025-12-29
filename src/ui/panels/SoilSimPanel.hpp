#pragma once
#include "imgui.h"
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"

namespace UI::Panels {

class PatchAnalysisPanel; // Forward declaration

class SoilSimPanel {
public:
    void drawScorpan(bool* open);
    void drawSiBCS(bool* open);
    
    void setPatchAnalysisPanel(PatchAnalysisPanel* panel) { patchAnalysisPanel_ = panel; }

private:
    
private:
    Core::Domain::Soils::ScorpanParams params_;
    int visualizationLevel_ = 1; // 1=Order, 2=Suborder, 3=GreatGroup
    Core::Domain::Soils::SiBCSFilter filter_; // New
    PatchAnalysisPanel* patchAnalysisPanel_ = nullptr;
    float rasterCellSize_ = 1.0f;
    std::string rasterStatus_;
};

} // namespace UI::Panels
