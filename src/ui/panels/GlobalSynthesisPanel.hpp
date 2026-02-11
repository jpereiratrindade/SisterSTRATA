#pragma once

#include "application/Session.hpp"
#include "application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <mutex>
#include <string>

namespace UI::Panels {

/**
 * @brief UI Panel for Strategic Global Synthesis.
 * Analyzes consistency across all Observational Contexts.
 */
class GlobalSynthesisPanel {
public:
    GlobalSynthesisPanel() = default;

    void setSession(Application::Session* session);
    void draw(bool* open);
    void drawInline(const char* idSuffix = "WorkspaceGlobalSynthesis");

private:
    void drawTabContent(bool shouldOpenAiPopup);

    Application::Session* session_ = nullptr;

    // AI Analysis State
    bool showAiModal_ = false;
    bool aiRequestPending_ = false;
    bool aiResultReady_ = false; 
    std::mutex aiMutex_;
    Application::DTO::Cognitive::InterpretationSnapshotDTO lastAiSnapshot_; 
    Application::DTO::Cognitive::InterpretationSnapshotDTO stagedAiSnapshot_;
    std::string auditStatus_;
    bool lastAuditFailed_ = false;

    void drawAuditSection();
    void drawHistoryTab();
};

} // namespace UI::Panels
