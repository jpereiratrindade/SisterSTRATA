#include "AnalysisWorkspacePanel.hpp"
#include "NarrativePanel.hpp"
#include "DiscursiveSystemPanel.hpp"
#include "RecommendationTrajectoryPanel.hpp"
#include "GlobalSynthesisPanel.hpp"
#include "imgui.h"
#include "ui/components/NarrativeGraphWidget.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <set>
#include <cmath>

namespace {

static std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

static bool containsAny(const std::string& text, const std::vector<const char*>& tokens) {
    for (const char* token : tokens) {
        if (text.find(token) != std::string::npos) return true;
    }
    return false;
}

static void addUnique(std::vector<std::string>& list, const std::string& value) {
    if (value.empty()) return;
    if (std::find(list.begin(), list.end(), value) == list.end()) {
        list.push_back(value);
    }
}

} // namespace

namespace UI::Panels {

void AnalysisWorkspacePanel::setSession(Application::Session* session) {
    session_ = session;
    reportsDirty_ = true;
    lastNarrativeCount_ = 0;
    contextProfiles_.clear();
}

void AnalysisWorkspacePanel::setNarrativePanel(NarrativePanel* narrativePanel) {
    narrativePanel_ = narrativePanel;
}

void AnalysisWorkspacePanel::setDiscursivePanel(DiscursiveSystemPanel* discursivePanel) {
    discursivePanel_ = discursivePanel;
}

void AnalysisWorkspacePanel::setRecommendationTrajectoryPanel(RecommendationTrajectoryPanel* recommendationTrajectoryPanel) {
    recommendationTrajectoryPanel_ = recommendationTrajectoryPanel;
}

void AnalysisWorkspacePanel::setGlobalSynthesisPanel(GlobalSynthesisPanel* globalSynthesisPanel) {
    globalSynthesisPanel_ = globalSynthesisPanel;
}

void AnalysisWorkspacePanel::setContextToolToggles(bool* showNarrativePanel,
                                                   bool* showDiscursivePanel,
                                                   bool* showRecommendationPanel,
                                                   bool* showGlobalSynthesisPanel,
                                                   bool* showTimeline,
                                                   bool* showPatchAnalysis) {
    showNarrativePanel_ = showNarrativePanel;
    showDiscursivePanel_ = showDiscursivePanel;
    showRecommendationPanel_ = showRecommendationPanel;
    showGlobalSynthesisPanel_ = showGlobalSynthesisPanel;
    showTimeline_ = showTimeline;
    showPatchAnalysis_ = showPatchAnalysis;
}

void AnalysisWorkspacePanel::setWorld3DAvailable(bool available) {
    world3DAvailable_ = available;
}

void AnalysisWorkspacePanel::draw(bool* open) {
    if (!open || !*open) return;

    if (!session_) {
        ImGui::SetNextWindowSize(ImVec2(1080, 720), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("STRATA - Analysis Workspace", open)) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: No Session Connected");
        }
        ImGui::End();
        return;
    }

    const std::string projectRoot = session_->getProjectRoot();
    if (projectRoot != lastProjectRoot_) {
        lastProjectRoot_ = projectRoot;
        reportsDirty_ = true;
        selectedReportIndex_ = -1;
    }

    refreshReports();
    ensureProfiles();

    ImGui::SetNextWindowSize(ImVec2(1280, 760), ImGuiCond_FirstUseEver);

    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("STRATA - Analysis Workspace", open, windowFlags)) {
        ImGui::TextDisabled("Analysis Workspace (observational, read-only)");
        ImGui::Separator();

        const float totalHeight = ImGui::GetContentRegionAvail().y;
        if (ImGui::BeginTabBar("AnalysisWorkspaceTabs")) {
            if (ImGui::BeginTabItem("Contexto Narrativo")) {
                drawContextAnalysisTab(totalHeight);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Contexto Discursivo")) {
                drawDiscursiveContextTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Trajectory")) {
                drawTrajectoryContextTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Recomendacoes")) {
                drawRecommendationContextTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void AnalysisWorkspacePanel::drawContextAnalysisTab(float totalHeight) {
    if (ImGui::BeginTabBar("NarrativeWorkspaceSubTabs")) {
        if (ImGui::BeginTabItem("Workspace Synthesis")) {
            drawNarrativeSynthesisWorkspace(totalHeight);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Narrative Observation Context")) {
            if (narrativePanel_) {
                narrativePanel_->drawInline("WorkspaceNarrativeContext");
            } else {
                ImGui::TextDisabled("Narrative panel unavailable.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void AnalysisWorkspacePanel::drawNarrativeSynthesisWorkspace(float totalHeight) {
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float totalWidth = ImGui::GetContentRegionAvail().x;
    const float leftWidth = std::max(260.0f, totalWidth * 0.26f);
    const float rightWidth = std::max(280.0f, totalWidth * 0.26f);
    const float centerWidth = std::max(360.0f, totalWidth - leftWidth - rightWidth - spacing * 2.0f);

    ImGui::BeginChild("AnalysisLeftPanel", ImVec2(leftWidth, totalHeight), true);
    drawLeftPanel(totalHeight);
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("AnalysisCenterPanel", ImVec2(centerWidth, totalHeight), true);
    drawCenterPanel(totalHeight);
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("AnalysisRightPanel", ImVec2(0.0f, totalHeight), true);
    drawRightPanel(totalHeight);
    ImGui::EndChild();
}

void AnalysisWorkspacePanel::drawDiscursiveContextTab() {
    if (discursivePanel_) {
        discursivePanel_->drawInline("WorkspaceDiscursiveContext");
        return;
    }

    ImGui::TextDisabled("Discursive panel unavailable.");
}

void AnalysisWorkspacePanel::drawTrajectoryContextTab() {
    if (recommendationTrajectoryPanel_) {
        recommendationTrajectoryPanel_->drawInline("WorkspaceTrajectoryContext");
    } else {
        ImGui::TextDisabled("Recommendation trajectory panel unavailable.");
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Acoes 3D auxiliares", ImGuiTreeNodeFlags_DefaultOpen)) {
        size_t snapshotCount = 0;
        if (session_) {
            snapshotCount = session_->getRecommendationTrajectoryDTO().snapshots.size();
        }
        ImGui::Text("Snapshots na trajectory: %zu", snapshotCount);

        if (ImGui::Button("Abrir Janela Trajectory Context") && showRecommendationPanel_) {
            *showRecommendationPanel_ = true;
        }

        ImGui::Spacing();
        if (!world3DAvailable_) {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "3D indisponivel neste modo de execucao.");
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Abrir Fourth Dimension") && showTimeline_) {
            *showTimeline_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Abrir Patch Analysis (CSV)") && showPatchAnalysis_) {
            *showPatchAnalysis_ = true;
        }
        if (!world3DAvailable_) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        ImGui::BulletText("3D pode permanecer como suporte");
        ImGui::BulletText("Dependencia explicita: UI/Application -> World3DService");
    }
}

void AnalysisWorkspacePanel::drawRecommendationContextTab() {
    if (globalSynthesisPanel_) {
        globalSynthesisPanel_->drawInline("WorkspaceRecommendations");
    } else {
        ImGui::TextDisabled("Global synthesis panel unavailable.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button("Abrir Janela Strategic Global Synthesis") && showGlobalSynthesisPanel_) {
        *showGlobalSynthesisPanel_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Abrir Janela Contexto Narrativo") && showNarrativePanel_) {
        *showNarrativePanel_ = true;
    }

    ImGui::Spacing();
    ImGui::BulletText("Sem inferencia causal automatica");
    ImGui::BulletText("Recomendacoes permanecem no nivel observacional/interpretativo");
}

void AnalysisWorkspacePanel::refreshReports() {
    if (!reportsDirty_) return;

    reportsDirty_ = false;
    reports_.clear();

    if (!session_) return;
    const std::filesystem::path reportDir = std::filesystem::path(session_->getProjectRoot()) / "reports" / "ingestion";
    if (!std::filesystem::exists(reportDir)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(reportDir)) {
        if (!entry.is_regular_file()) continue;
        const auto path = entry.path();
        if (path.extension() != ".json") continue;
        const std::string filename = path.filename().string();
        if (filename.find("IngestionSynthesisReport") != 0) continue;

        ReportEntry report;
        report.path = path.string();
        report.fileName = filename;
        report.isLatest = (filename.find("latest") != std::string::npos);

        try {
            std::ifstream in(path);
            if (in.is_open()) {
                in >> report.report;
                report.generatedAt = report.report.value("generatedAt", "");
                report.trigger = report.report.value("trigger", "");
                report.sourcePath = report.report.value("sourcePath", "");
                const auto summary = report.report.value("summary", nlohmann::json::object());
                report.bundlesDetected = summary.value("bundlesDetected", 0);
                report.bundlesIngested = summary.value("bundlesIngested", 0);
                report.standaloneFiles = summary.value("standaloneFiles", 0);
                report.coveragePercent = summary.value("coveragePercent", 0.0);
                report.parseOk = true;
            }
        } catch (...) {
            report.parseOk = false;
        }

        reports_.push_back(std::move(report));
    }

    std::sort(reports_.begin(), reports_.end(), [](const ReportEntry& a, const ReportEntry& b) {
        if (a.isLatest != b.isLatest) return a.isLatest;
        return a.generatedAt > b.generatedAt;
    });

    if (!reports_.empty() && selectedReportIndex_ < 0) {
        selectedReportIndex_ = 0;
    }
}

void AnalysisWorkspacePanel::ensureProfiles() {
    if (!session_) return;
    auto history = session_->getNarrativeHistoryDTO();
    if (history.size() == lastNarrativeCount_) return;

    lastNarrativeCount_ = history.size();
    contextProfiles_.clear();

    for (const auto& dto : history) {
        std::string sourceId = dto.source.sourceId.empty() ? "unknown_source" : dto.source.sourceId;
        auto& profile = contextProfiles_[sourceId];

        for (const auto& axis : dto.axes) {
            if (!axis.label.empty()) profile.processCounts[axis.label] += 1;
            if (!axis.description.empty()) addUnique(profile.descriptions, axis.description);
        }

        for (const auto& [key, value] : dto.metadata) {
            if (key == "iw.context" || key == "iw.observation" || key == "iw.evidenceSnippet") {
                addUnique(profile.descriptions, value);
            }
        }

        std::vector<std::string> scaleHints;
        auto itCtx = dto.metadata.find("iw.context");
        if (itCtx != dto.metadata.end()) scaleHints.push_back(itCtx->second);
        auto itObs = dto.metadata.find("iw.observation");
        if (itObs != dto.metadata.end()) scaleHints.push_back(itObs->second);
        for (const auto& axis : dto.axes) {
            if (!axis.label.empty()) scaleHints.push_back(axis.label);
            if (!axis.description.empty()) scaleHints.push_back(axis.description);
        }

        bool foundAny = false;
        for (const auto& hint : scaleHints) {
            const NarrativeScale scale = inferScaleFromText(hint);
            if (scale != NarrativeScale::Unknown) {
                profile.scaleCounts[scale] += 1;
                foundAny = true;
            }
        }
        if (!foundAny) {
            profile.scaleCounts[NarrativeScale::Unknown] += 1;
        }
    }

    for (auto& [_, profile] : contextProfiles_) {
        int nonUnknown = 0;
        for (const auto& [scale, _count] : profile.scaleCounts) {
            if (scale == NarrativeScale::Unknown || scale == NarrativeScale::MultiScale) continue;
            nonUnknown += 1;
        }
        if (nonUnknown > 1) {
            profile.scaleCounts[NarrativeScale::MultiScale] += 1;
        }
    }
}

const AnalysisWorkspacePanel::ReportEntry* AnalysisWorkspacePanel::getSelectedReport() const {
    if (selectedReportIndex_ < 0 || selectedReportIndex_ >= static_cast<int>(reports_.size())) return nullptr;
    return &reports_[selectedReportIndex_];
}

nlohmann::json AnalysisWorkspacePanel::getGraphJson() const {
    if (const auto* report = getSelectedReport()) {
        if (report->report.contains("narrativeContextGraph")) {
            return report->report.value("narrativeContextGraph", nlohmann::json::object());
        }
    }
    if (session_) {
        return session_->getNarrativeContextGraph();
    }
    return nlohmann::json::object();
}

void AnalysisWorkspacePanel::drawLeftPanel(float height) {
    (void)height;

    ImGui::Text("Ingestoes");
    ImGui::Separator();

    if (ImGui::Button("Rescan")) {
        reportsDirty_ = true;
        refreshReports();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Total: %zu", reports_.size());

    ImGui::Spacing();
    ImGui::BeginChild("IngestionList", ImVec2(0.0f, 140.0f), true);
    if (reports_.empty()) {
        ImGui::TextDisabled("Nenhuma ingestao registrada.");
    } else {
        for (size_t i = 0; i < reports_.size(); ++i) {
            const auto& report = reports_[i];
            const std::string label = report.generatedAt.empty() ? report.fileName : report.generatedAt;
            if (ImGui::Selectable(label.c_str(), selectedReportIndex_ == static_cast<int>(i))) {
                selectedReportIndex_ = static_cast<int>(i);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Trigger: %s", report.trigger.empty() ? "unknown" : report.trigger.c_str());
                ImGui::Text("Source: %s", report.sourcePath.empty() ? "unknown" : report.sourcePath.c_str());
                ImGui::EndTooltip();
            }
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Relatorios de sintese");
    ImGui::BeginChild("ReportsList", ImVec2(0.0f, 160.0f), true);
    if (reports_.empty()) {
        ImGui::TextDisabled("Nenhum relatorio encontrado.");
    } else {
        for (size_t i = 0; i < reports_.size(); ++i) {
            const auto& report = reports_[i];
            std::string label = report.fileName;
            if (report.isLatest) label += " (latest)";
            if (!report.parseOk) label += " [parse error]";
            if (ImGui::Selectable(label.c_str(), selectedReportIndex_ == static_cast<int>(i))) {
                selectedReportIndex_ = static_cast<int>(i);
            }
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Filtros");
    ImGui::Separator();

    ImGui::Text("Dimensao");
    ImGui::Checkbox("Ecologica", &filterEcological_);
    ImGui::Checkbox("Produtiva", &filterProductive_);
    ImGui::Checkbox("Social", &filterSocial_);
    ImGui::Checkbox("Mista", &filterMixed_);

    ImGui::Spacing();
    ImGui::Text("Escala (derivada; filtro inativo)");
    bool dummyScale = false;
    ImGui::BeginDisabled();
    ImGui::Checkbox("Local", &dummyScale);
    ImGui::Checkbox("Parcela", &dummyScale);
    ImGui::Checkbox("Campo", &dummyScale);
    ImGui::Checkbox("Regional", &dummyScale);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Text("Projeto");
    ImGui::BeginDisabled();
    ImGui::TextWrapped("%s", lastProjectRoot_.empty() ? "[sem projeto]" : lastProjectRoot_.c_str());
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Detalhes rapidos");

    if (const auto* report = getSelectedReport()) {
        ImGui::Text("Trigger: %s", report->trigger.empty() ? "unknown" : report->trigger.c_str());
        ImGui::Text("Bundles: %d/%d", report->bundlesIngested, report->bundlesDetected);
        ImGui::Text("Coverage: %.2f%%", report->coveragePercent);
    }
}

void AnalysisWorkspacePanel::drawCenterPanel(float height) {
    (void)height;
    ImGui::Text("Grafo Narrativo");
    ImGui::Separator();

    ImGui::Checkbox("Narrativas", &showNarratives_);
    ImGui::SameLine();
    ImGui::Checkbox("Contextos", &showContexts_);
    ImGui::SameLine();
    ImGui::Checkbox("Processos", &showProcesses_);

    ImGui::Spacing();
    ImGui::BulletText("Proximidade narrativa (nao causal)");
    ImGui::BulletText("Sem inferencia de estados ou resiliencia");
    ImGui::BulletText("Leitura observacional");

    ImGui::Separator();

    drawNarrativeGraphWidget(getGraphJson());
}

void AnalysisWorkspacePanel::drawRightPanel(float height) {
    (void)height;
    ImGui::Text("Contexto Selecionado");
    ImGui::Separator();
    drawContextDetails();
}

void AnalysisWorkspacePanel::drawNarrativeGraphWidget(const nlohmann::json& graph) {
    if (!showContexts_) {
        ImGui::TextDisabled("Contextos ocultos. Reative o toggle para visualizar.");
        return;
    }

    // Build process profiles map for widget tooltip enrichment
    UI::Components::NarrativeGraphOptions::ProcessMap processMap;
    for (const auto& [contextId, profile] : contextProfiles_) {
        processMap[contextId] = profile.processCounts;
    }

    UI::Components::NarrativeGraphOptions options;
    options.filterEcological = &filterEcological_;
    options.filterProductive = &filterProductive_;
    options.filterSocial = &filterSocial_;
    options.filterMixed = &filterMixed_;
    options.showNarrativeCounts = showNarratives_;
    options.scaleRadiusByCount = showNarratives_;
    options.processProfiles = &processMap;
    options.showProcesses = showProcesses_;

    UI::Components::NarrativeGraphWidget::draw(graph, graphState_, "Workspace", options);

    if (showNarratives_) {
        ImGui::BulletText("Proximidade narrativa (nao causal)");
    }
    ImGui::BulletText("Sem causalidade");
}

void AnalysisWorkspacePanel::drawContextDetails() {
    if (graphState_.selectedNodeId.empty()) {
        ImGui::TextDisabled("Selecione um contexto no grafo para ver detalhes.");
        return;
    }

    auto itProfile = contextProfiles_.find(graphState_.selectedNodeId);
    if (itProfile == contextProfiles_.end()) {
        ImGui::TextDisabled("Contexto sem dados narrativos carregados.");
        return;
    }

    const auto& profile = itProfile->second;

    ImGui::Text("Descricao");
    if (profile.descriptions.empty()) {
        ImGui::TextDisabled("Sem descricao textual registrada.");
    } else {
        int shown = 0;
        for (const auto& desc : profile.descriptions) {
            ImGui::BulletText("%s", desc.c_str());
            if (++shown >= 6) break;
        }
    }

    ImGui::Spacing();
    ImGui::Text("Processos associados");
    if (profile.processCounts.empty()) {
        ImGui::TextDisabled("Sem processos narrados.");
    } else {
        std::vector<std::pair<std::string, int>> processes(profile.processCounts.begin(), profile.processCounts.end());
        std::sort(processes.begin(), processes.end(), [](const auto& a, const auto& b) {
            if (a.second == b.second) return a.first < b.first;
            return a.second > b.second;
        });
        int shown = 0;
        for (const auto& [label, count] : processes) {
            ImGui::BulletText("%s (N=%d)", label.c_str(), count);
            if (++shown >= 8) break;
        }
    }

    ImGui::Spacing();
    ImGui::Text("Escalas (derivadas)");
    if (profile.scaleCounts.empty()) {
        ImGui::TextDisabled("Unknown");
    } else {
        for (const auto& [scale, count] : profile.scaleCounts) {
            ImGui::BulletText("%s (N=%d)", labelForScale(scale), count);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Limites epistemologicos");

    if (const auto* report = getSelectedReport()) {
        const auto status = report->report.value("epistemicStatus", nlohmann::json::object());
        ImGui::Text("Type: %s", status.value("type", "unknown").c_str());
        ImGui::Text("allowsResilienceInference: %s", status.value("allowsResilienceInference", false) ? "true" : "false");
        ImGui::Text("requiresSpatialTemporalData: %s", status.value("requiresSpatialTemporalData", true) ? "true" : "false");
        const std::string notes = status.value("notes", "");
        if (!notes.empty()) {
            ImGui::TextWrapped("Notes: %s", notes.c_str());
        }
    } else {
        ImGui::Text("Type: observational_synthesis");
        ImGui::Text("allowsResilienceInference: false");
        ImGui::Text("requiresSpatialTemporalData: true");
        ImGui::TextWrapped("Notes: Report is descriptive. No causal or prescriptive inference is performed.");
    }
}

AnalysisWorkspacePanel::NarrativeScale AnalysisWorkspacePanel::inferScaleFromText(const std::string& text) {
    if (text.empty()) return NarrativeScale::Unknown;
    const std::string lower = toLower(text);

    if (containsAny(lower, {"regional", "bacia", "paisagem", "macro"})) return NarrativeScale::Regional;
    if (containsAny(lower, {"campo", "fazenda", "field"})) return NarrativeScale::Field;
    if (containsAny(lower, {"parcela", "plot", "quadrante"})) return NarrativeScale::Plot;
    if (containsAny(lower, {"local", "micro", "ponto", "perfil"})) return NarrativeScale::Local;

    return NarrativeScale::Unknown;
}

const char* AnalysisWorkspacePanel::labelForScale(NarrativeScale scale) {
    switch (scale) {
        case NarrativeScale::Local: return "Local";
        case NarrativeScale::Plot: return "Parcela";
        case NarrativeScale::Field: return "Campo";
        case NarrativeScale::Regional: return "Regional";
        case NarrativeScale::MultiScale: return "MultiScale";
        case NarrativeScale::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace UI::Panels
