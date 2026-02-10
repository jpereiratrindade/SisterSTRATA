#pragma once

#include "application/Session.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace UI::Panels {

class AnalysisWorkspacePanel {
public:
    AnalysisWorkspacePanel() = default;

    void setSession(Application::Session* session);
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
        std::map<std::string, int> processCounts;
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

    float graphMinSimilarity_ = 0.35f;
    int graphTopKPerNode_ = 3;
    bool graphShowLabels_ = false;
    bool graphHideIsolated_ = false;
    bool graphFocusSelected_ = false;
    std::string selectedGraphNodeId_;

    void refreshReports();
    void ensureProfiles();

    const ReportEntry* getSelectedReport() const;
    nlohmann::json getGraphJson() const;

    void drawLeftPanel(float height);
    void drawCenterPanel(float height);
    void drawRightPanel(float height);

    void drawNarrativeGraph(const nlohmann::json& graph);
    void drawContextDetails();

    static NarrativeScale inferScaleFromText(const std::string& text);
    static const char* labelForScale(NarrativeScale scale);
};

} // namespace UI::Panels
