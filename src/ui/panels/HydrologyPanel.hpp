#pragma once

#include <string>
#include "ui/components/FileSelector.hpp"
#include "application/services/World3DService.hpp"

namespace UI::Panels {

/**
 * @brief Hydrology controls and reporting UI.
 */
class HydrologyPanel {
public:
    /**
     * @brief Render the hydrology panel.
     */
    void draw(bool* open);

private:
    UI::Components::FileSelector reportFileSelector_{"Hydrology Report"};
    UI::Components::FileSelector basinFileSelector_{"Basin Boundaries"};
    bool showReportDialog_ = false;
    bool showBasinDialog_ = false;
    float boundaryPointSize_ = 4.0f;

    bool showDrainage_ = false;
    bool showWatersheds_ = false;
    bool showBasinOutlines_ = false;
    float drainageIntensity_ = 0.2f;
    float streamThreshold_ = 100.0f;

    Application::Services::World3DService::DrainageStats lastDrainageStats_;
    bool hasDrainageStats_ = false;

    Application::Services::World3DService::HydrologyStats lastHydrologyStats_;
    bool hasHydrologyStats_ = false;
    std::string statusMessage_;
};

} // namespace UI::Panels
