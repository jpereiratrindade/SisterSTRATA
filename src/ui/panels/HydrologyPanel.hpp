#pragma once

#include <string>
#include "ui/components/FileSelector.hpp"
#include "world3d/World3D.hpp"

namespace UI::Panels {

class HydrologyPanel {
public:
    void draw(bool* open);

private:
    UI::Components::FileSelector reportFileSelector_{"Hydrology Report"};
    bool showReportDialog_ = false;

    bool showDrainage_ = false;
    bool showWatersheds_ = false;
    bool showBasinOutlines_ = false;
    float drainageIntensity_ = 0.2f;
    float streamThreshold_ = 100.0f;

    World3D::DrainageStats lastDrainageStats_;
    bool hasDrainageStats_ = false;

    World3D::HydrologyStats lastHydrologyStats_;
    bool hasHydrologyStats_ = false;
    std::string statusMessage_;
};

} // namespace UI::Panels
