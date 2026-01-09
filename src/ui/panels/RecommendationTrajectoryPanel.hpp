#pragma once

#include "application/Session.hpp"
#include "ui/components/FileSelector.hpp"
#include <string>
#include <vector>
#include <map>

namespace UI::Panels {

/**
 * @brief UI Panel for the Recommendation Trajectory Context.
 */
class RecommendationTrajectoryPanel {
public:
    RecommendationTrajectoryPanel() = default;

    void setSession(Application::Session* session);
    void draw(bool* open);

private:
    Application::Session* session_ = nullptr;

    // Trajectory setup
    char inputTrajectoryId_[64] = "";
    char inputMetadataKey_[64] = "";
    char inputMetadataValue_[128] = "";
    std::map<std::string, std::string> pendingMetadata_;

    // Snapshot form
    char inputSnapshotId_[64] = "";
    char inputRecommendationText_[1024] = "";
    char inputIntendedAction_[256] = "";
    char inputExpectedOutcome_[256] = "";

    int inputSourceType_ = 0;
    char inputSourceId_[64] = "";
    char inputSourceDate_[64] = "";
    char inputSourceAuthor_[64] = "";

    int inputTemporalCategory_ = 3;
    char inputTemporalLabel_[128] = "";

    char inputContextCondition_[128] = "";
    std::vector<std::string> contextConditions_;

    UI::Components::FileSelector importSelector_{"Import Recommendation JSON"};
    UI::Components::FileSelector exportSelector_{"Export Recommendation JSON"};
    bool showImportDialog_ = false;
    bool showExportDialog_ = false;
    std::string lastImportPath_ = "assets/data/";
    std::string lastExportPath_ = "assets/data/";

    void drawTrajectoryConfig();
    void drawSnapshotForm();
    void drawSnapshotList();
    void loadIntoForm(const Application::DTO::RecommendationSnapshotDTO& dto);

    // Edit State
    bool isEditing_ = false;
    std::string editingId_;

    // -- AI Analysis State --
    bool showAiModal_ = false;
    bool aiRequestPending_ = false;
    bool aiResultReady_ = false;
    std::mutex aiMutex_;
    Application::DTO::Cognitive::InterpretationSnapshotDTO lastAiSnapshot_;
    Application::DTO::Cognitive::InterpretationSnapshotDTO stagedAiSnapshot_;
};

} // namespace UI::Panels
