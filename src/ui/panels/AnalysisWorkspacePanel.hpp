#pragma once

#include "application/Session.hpp"
#include <nlohmann/json.hpp>
#include "ui/components/NarrativeGraphWidget.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace UI::Panels {

class NarrativePanel;
class DiscursiveSystemPanel;
class RecommendationTrajectoryPanel;
class GlobalSynthesisPanel;

class AnalysisWorkspacePanel {
public:
    AnalysisWorkspacePanel() = default;

    void setSession(Application::Session* session);
    void setNarrativePanel(NarrativePanel* narrativePanel);
    void setDiscursivePanel(DiscursiveSystemPanel* discursivePanel);
    void setRecommendationTrajectoryPanel(RecommendationTrajectoryPanel* recommendationTrajectoryPanel);
    void setGlobalSynthesisPanel(GlobalSynthesisPanel* globalSynthesisPanel);
    void setContextToolToggles(bool* showNarrativePanel,
                               bool* showDiscursivePanel,
                               bool* showRecommendationPanel,
                               bool* showGlobalSynthesisPanel,
                               bool* showTimeline,
                               bool* showPatchAnalysis);
    void setWorld3DAvailable(bool available);
    void draw(bool* open);

private:
    enum class NarrativeScale {
        Unknown,
        Local,
        Plot,
        Field,
        Regional,
        MultiScale
    };

    struct ReportEntry {
        std::string path;
        std::string fileName;
        std::string generatedAt;
        std::string trigger;
        std::string sourcePath;
        int bundlesDetected = 0;
        int bundlesIngested = 0;
        int standaloneFiles = 0;
        double coveragePercent = 0.0;
        bool parseOk = false;
        bool isLatest = false;
        nlohmann::json report;
    };

    struct ContextProfile {
        std::vector<std::string> descriptions;
        std::unordered_map<std::string, int> processCounts;
        std::map<NarrativeScale, int> scaleCounts;
    };

    Application::Session* session_ = nullptr;

    std::string lastProjectRoot_;
    std::vector<ReportEntry> reports_;
    int selectedReportIndex_ = -1;
    bool reportsDirty_ = true;

    std::unordered_map<std::string, ContextProfile> contextProfiles_;
    size_t lastNarrativeCount_ = 0;

    bool filterEcological_ = true;
    bool filterProductive_ = true;
    bool filterSocial_ = true;
    bool filterMixed_ = true;

    bool showNarratives_ = true;
    bool showContexts_ = true;
    bool showProcesses_ = false;

    UI::Components::NarrativeGraphState graphState_;

    bool world3DAvailable_ = true;
    NarrativePanel* narrativePanel_ = nullptr;
    DiscursiveSystemPanel* discursivePanel_ = nullptr;
    RecommendationTrajectoryPanel* recommendationTrajectoryPanel_ = nullptr;
    GlobalSynthesisPanel* globalSynthesisPanel_ = nullptr;
    bool* showNarrativePanel_ = nullptr;
    bool* showDiscursivePanel_ = nullptr;
    bool* showRecommendationPanel_ = nullptr;
    bool* showGlobalSynthesisPanel_ = nullptr;
    bool* showTimeline_ = nullptr;
    bool* showPatchAnalysis_ = nullptr;

    void refreshReports();
    void ensureProfiles();

    const ReportEntry* getSelectedReport() const;
    nlohmann::json getGraphJson() const;

    void drawLeftPanel(float height);
    void drawCenterPanel(float height);
    void drawRightPanel(float height);
    void drawNarrativeSynthesisWorkspace(float totalHeight);
    void drawContextAnalysisTab(float totalHeight);
    void drawDiscursiveContextTab();
    void drawTrajectoryContextTab();
    void drawRecommendationContextTab();

    void drawNarrativeGraphWidget(const nlohmann::json& graph);
    void drawContextDetails();

    static NarrativeScale inferScaleFromText(const std::string& text);
    static const char* labelForScale(NarrativeScale scale);
};

} // namespace UI::Panels
