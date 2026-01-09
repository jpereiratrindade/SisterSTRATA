#pragma once

#include "application/Session.hpp"
#include "ui/components/FileSelector.hpp"
#include "application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace UI::Panels {

/**
 * @brief UI Panel for the Discursive System Context.
 */
class DiscursiveSystemPanel {
public:
    DiscursiveSystemPanel() = default;

    void setSession(Application::Session* session);
    void draw(bool* open);

private:
    Application::Session* session_ = nullptr;

    // Edit State
    bool isEditing_ = false;
    std::string editingId_;

    // Form state
    char inputSystemId_[64] = "";

    int inputSourceType_ = 0;
    char inputSourceId_[64] = "";
    char inputSourceDate_[64] = "";
    char inputSourceAuthor_[64] = "";

    int inputTemporalCategory_ = 3;
    char inputTemporalLabel_[128] = "";

    char inputProblem_[256] = "";
    char inputAction_[256] = "";
    char inputMechanism_[256] = "";
    char inputEffect_[256] = "";

    char inputMetadataKey_[64] = "";
    char inputMetadataValue_[128] = "";

    std::vector<Application::DTO::SourceReferenceDTO> sourceReferences_;
    std::vector<std::string> declaredProblems_;
    std::vector<std::string> declaredActions_;
    std::vector<std::string> allegedMechanisms_;
    std::vector<std::string> expectedEffects_;
    std::map<std::string, std::string> metadata_;

    UI::Components::FileSelector importSelector_{"Import Discursive JSON"};
    UI::Components::FileSelector exportSelector_{"Export Discursive JSON"};
    bool showImportDialog_ = false;
    bool showExportDialog_ = false;
    std::string lastImportPath_ = "assets/data/";
    std::string lastExportPath_ = "assets/data/";

    void drawIngestionForm();
    void drawSystemList();
    void loadIntoForm(const Application::DTO::DiscursiveSystemDTO& dto);

    // AI Analysis State
    bool showAiModal_ = false;
    bool aiRequestPending_ = false;
    bool aiResultReady_ = false; // Thread-safe signal
    std::mutex aiMutex_;
    Application::DTO::Cognitive::InterpretationSnapshotDTO lastAiSnapshot_;
};

} // namespace UI::Panels
