#pragma once

#include "ui/panels/VegetationDeclarationPanel.hpp"
#include "application/Session.hpp"
#include "application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include "application/ports/ILLMService.hpp"
#include <string>
#include <mutex>

namespace Core::Domain::FourthDimension {
class Trajectory;
}

namespace Core::Domain::SpatialPattern {
struct AnalysisResult;
}

namespace UI::Panels {

class TimelinePanel {
public:
    void setSession(Application::Session* session) {
        session_ = session;
    }
    
    void setDependencies(Core::Domain::FourthDimension::Trajectory* trajectory, 
                         VegetationDeclarationPanel* vegPanel,
                         Application::Ports::ILLMService* llmService) {
        trajectory_ = trajectory;
        vegPanel_ = vegPanel;
        llmService_ = llmService;
    }

    void draw(bool* open);

private:
    Core::Domain::FourthDimension::Trajectory* trajectory_ = nullptr;
    VegetationDeclarationPanel* vegPanel_ = nullptr;
    Application::Ports::ILLMService* llmService_ = nullptr;
    Application::Session* session_ = nullptr;
    
    char projectRootName_[64] = "projeto_01";

    // UI State
    int selectedSliceIndex_ = -1;
    int compareSliceA_ = -1;
    int compareSliceB_ = -1;
    float lastCoherenceMean_ = -1.0f;
    std::string coherenceStatus_;
    bool ghostMode_ = false; 

    // Cognitive Insight State
    float insightWindowHeight_ = 150.0f;
    std::string hermeneuticInsight_;
    std::string trajectoryInsight_;
    std::string hermeneuticContext_;  // Provenance data
    std::string trajectoryContext_;   // Provenance data
    bool hermeneuticInProgress_ = false;
    bool trajectoryInProgress_ = false;
    std::string llmErrorMessage_;
    std::mutex insightMutex_;

    // -- Unified AI State --
    bool showAiModal_ = false;
    bool aiRequestPending_ = false;
    bool aiResultReady_ = false;
    Application::DTO::Cognitive::InterpretationSnapshotDTO lastAiSnapshot_;
    Application::DTO::Cognitive::InterpretationSnapshotDTO stagedAiSnapshot_;
    
    void saveAnalysisToFile(const std::string& content, const std::string& type);
    
    // Patch Trajectory State
    int selectedPatchId_ = -1;
    int lastPatchCount_ = 0;
    Core::Domain::SpatialPattern::AnalysisResult lastPatchAnalysis_;
    std::string patchTrajectorySummary_;

    // Aggregated state summary
    std::string getClassDistribution(const Core::Domain::FourthDimension::TimeSlice& slice);

    // Helper to visualize ghost
    void applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice);
};

} // namespace UI::Panels
