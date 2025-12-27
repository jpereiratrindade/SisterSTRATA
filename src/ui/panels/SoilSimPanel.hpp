#pragma once
#include "imgui.h"
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"

namespace UI::Panels {

class SoilSimPanel {
public:
    void drawScorpan(bool* open);
    void drawSiBCS(bool* open);
    
private:
    Core::Domain::Soils::ScorpanParams params_;
    int visualizationLevel_ = 1; // 1=Order, 2=Suborder, 3=GreatGroup
    Core::Domain::Soils::SiBCSFilter filter_; // New
};

} // namespace UI::Panels
