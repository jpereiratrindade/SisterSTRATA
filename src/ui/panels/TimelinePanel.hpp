#pragma once

#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "ui/panels/VegetationDeclarationPanel.hpp"
#include <string>

namespace UI::Panels {

class TimelinePanel {
public:
    void setDependencies(Core::Domain::FourthDimension::Trajectory* trajectory, 
                         const VegetationDeclarationPanel* vegPanel) {
        trajectory_ = trajectory;
        vegPanel_ = vegPanel;
    }

    void draw(bool* open);

private:
    Core::Domain::FourthDimension::Trajectory* trajectory_ = nullptr;
    const VegetationDeclarationPanel* vegPanel_ = nullptr;
    
    // UI State
    int selectedSliceIndex_ = -1;
    bool ghostMode_ = false; 
    
    // Helper to visualize ghost
    void applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice);
};

} // namespace UI::Panels
