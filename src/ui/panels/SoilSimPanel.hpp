#pragma once
#include "imgui.h"
#include "core/domain/soils/Scorpan.hpp"

namespace UI::Panels {

class SoilSimPanel {
public:
    void draw(bool* open);
    
private:
    Core::Domain::Soils::ScorpanParams params_;
};

} // namespace UI::Panels
