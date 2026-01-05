#pragma once

#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "ui/panels/VegetationDeclarationPanel.hpp"
#include "application/ports/ILLMService.hpp"
#include <string>
#include <mutex>

namespace UI::Panels {

class TimelinePanel {
public:
    void setDependencies(Core::Domain::FourthDimension::Trajectory* trajectory, 
                         const VegetationDeclarationPanel* vegPanel,
                         Application::Ports::ILLMService* llmService) {
        trajectory_ = trajectory;
        vegPanel_ = vegPanel;
        llmService_ = llmService;
    }

    void draw(bool* open);

private:
    Core::Domain::FourthDimension::Trajectory* trajectory_ = nullptr;
    const VegetationDeclarationPanel* vegPanel_ = nullptr;
    Application::Ports::ILLMService* llmService_ = nullptr;
    
    // UI State
    int selectedSliceIndex_ = -1;
    int compareSliceA_ = -1;
    int compareSliceB_ = -1;
    float lastCoherenceMean_ = -1.0f;
    std::string coherenceStatus_;
    bool ghostMode_ = false; 

    // Cognitive Insight State
    float insightWindowHeight_ = 150.0f;
    std::mutex insightMutex_;
    
    void saveAnalysisToFile();
    
    // Helper to visualize ghost
    void applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice);
};

} // namespace UI::Panels
