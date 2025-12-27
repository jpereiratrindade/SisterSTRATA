#pragma once
#include "imgui.h"
#include "core/domain/soils/Scorpan.hpp"

namespace UI::Panels {

class SoilSimPanel {
public:
    void drawScorpan(bool* open);
    void drawSiBCS(bool* open);
    
private:
    Core::Domain::Soils::ScorpanParams params_;
    int visualizationLevel_ = 1; // 1=Order, 2=Suborder, 3=GreatGroup
};

} // namespace UI::Panels
