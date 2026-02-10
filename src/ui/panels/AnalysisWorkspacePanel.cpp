#include "AnalysisWorkspacePanel.hpp"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <set>
#include <cmath>

namespace {

struct NarrativeGraphNodeUI {
    std::string id;
    std::string label;
    std::string dominantDimension;
    std::string dominantIntent;
    int narrativeCount = 0;
    std::vector<std::string> topTokens;
    std::vector<std::string> artifactIds;
    std::vector<std::string> observationIds;
    ImVec2 position {};
    float radius = 12.0f;
};

struct NarrativeGraphEdgeUI {
    int source = -1;
    int target = -1;
    float similarity = 0.0f;
    float distance = 1.0f;
    int sharedTokens = 0;
};

static ImU32 colorForDimension(const std::string& dimension) {
    if (dimension == "ecological") return IM_COL32(90, 190, 110, 220);
    if (dimension == "productive") return IM_COL32(225, 170, 70, 220);
    if (dimension == "social") return IM_COL32(95, 145, 225, 220);
    return IM_COL32(170, 170, 185, 210);
}

static const char* labelForDimension(const std::string& dimension) {
    if (dimension == "ecological") return "Ecological";
    if (dimension == "productive") return "Productive";
    if (dimension == "social") return "Social";
    return "Mixed";
}

static void drawDimensionLegendItem(const char* id, ImU32 color, const char* label) {
    const ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
    ImGui::ColorButton(id, c, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(14, 14));
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
}

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

        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float totalWidth = ImGui::GetContentRegionAvail().x;
        const float totalHeight = ImGui::GetContentRegionAvail().y;
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
    ImGui::End();
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

    drawNarrativeGraph(getGraphJson());
}

void AnalysisWorkspacePanel::drawRightPanel(float height) {
    (void)height;
    ImGui::Text("Contexto Selecionado");
    ImGui::Separator();
    drawContextDetails();
}

void AnalysisWorkspacePanel::drawNarrativeGraph(const nlohmann::json& graph) {
    if (!showContexts_) {
        ImGui::TextDisabled("Contextos ocultos. Reative o toggle para visualizar.");
        return;
    }

    const auto nodeJson = graph.value("nodes", nlohmann::json::array());
    const auto edgeJson = graph.value("edges", nlohmann::json::array());

    std::vector<NarrativeGraphNodeUI> nodes;
    nodes.reserve(nodeJson.size());
    std::unordered_map<std::string, int> nodeIndex;
    for (const auto& n : nodeJson) {
        if (!n.is_object()) continue;

        NarrativeGraphNodeUI node;
        node.id = n.value("id", "");
        node.label = n.value("label", node.id);
        node.dominantDimension = n.value("dominantDimension", "mixed");
        node.dominantIntent = n.value("dominantIntent", "unknown");
        node.narrativeCount = n.value("narrativeCount", 0);

        if (n.contains("topTokens") && n["topTokens"].is_array()) {
            for (const auto& token : n["topTokens"]) {
                if (token.is_string()) node.topTokens.push_back(token.get<std::string>());
            }
        }
        if (n.contains("artifactIds") && n["artifactIds"].is_array()) {
            for (const auto& artifactId : n["artifactIds"]) {
                if (artifactId.is_string()) node.artifactIds.push_back(artifactId.get<std::string>());
            }
        }
        if (n.contains("observationIds") && n["observationIds"].is_array()) {
            for (const auto& observationId : n["observationIds"]) {
                if (observationId.is_string()) node.observationIds.push_back(observationId.get<std::string>());
            }
        }

        node.radius = showNarratives_
            ? 11.0f + std::sqrt(static_cast<float>(std::max(1, node.narrativeCount))) * 4.0f
            : 14.0f;

        nodeIndex[node.id] = static_cast<int>(nodes.size());
        nodes.push_back(std::move(node));
    }

    std::vector<NarrativeGraphEdgeUI> edges;
    edges.reserve(edgeJson.size());
    for (const auto& e : edgeJson) {
        if (!e.is_object()) continue;
        const std::string sourceId = e.value("source", "");
        const std::string targetId = e.value("target", "");
        auto itA = nodeIndex.find(sourceId);
        auto itB = nodeIndex.find(targetId);
        if (itA == nodeIndex.end() || itB == nodeIndex.end()) continue;

        NarrativeGraphEdgeUI edge;
        edge.source = itA->second;
        edge.target = itB->second;
        edge.similarity = e.value("similarity", 0.0f);
        edge.distance = e.value("distance", 1.0f);
        edge.sharedTokens = e.value("sharedTokens", 0);
        edges.push_back(edge);
    }

    auto normalizeDimension = [](std::string dim) {
        if (dim != "ecological" && dim != "productive" && dim != "social") {
            dim = "mixed";
        }
        return dim;
    };

    std::unordered_map<std::string, int> stableNodeLookup;
    stableNodeLookup.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        stableNodeLookup[nodes[i].id] = static_cast<int>(i);
    }
    if (!selectedGraphNodeId_.empty() && !stableNodeLookup.contains(selectedGraphNodeId_)) {
        selectedGraphNodeId_.clear();
    }

    std::vector<bool> edgeActive(edges.size(), false);
    for (size_t i = 0; i < edges.size(); ++i) {
        edgeActive[i] = edges[i].similarity >= graphMinSimilarity_;
    }

    const int topK = std::max(0, graphTopKPerNode_);
    if (topK > 0) {
        std::vector<std::vector<std::pair<float, int>>> incident(nodes.size());
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            incident[edges[i].source].push_back({edges[i].similarity, static_cast<int>(i)});
            incident[edges[i].target].push_back({edges[i].similarity, static_cast<int>(i)});
        }

        std::vector<bool> keep(edges.size(), false);
        for (auto& list : incident) {
            std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
                if (a.first == b.first) return a.second < b.second;
                return a.first > b.first;
            });
            const int limit = std::min<int>(topK, static_cast<int>(list.size()));
            for (int i = 0; i < limit; ++i) {
                keep[list[i].second] = true;
            }
        }

        for (size_t i = 0; i < edges.size(); ++i) {
            edgeActive[i] = edgeActive[i] && keep[i];
        }
    }

    int selectedIndex = -1;
    if (!selectedGraphNodeId_.empty()) {
        auto it = stableNodeLookup.find(selectedGraphNodeId_);
        if (it != stableNodeLookup.end()) selectedIndex = it->second;
    }

    std::set<int> focusNodes;
    if (graphFocusSelected_ && selectedIndex >= 0) {
        focusNodes.insert(selectedIndex);
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            if (edges[i].source == selectedIndex || edges[i].target == selectedIndex) {
                focusNodes.insert(edges[i].source);
                focusNodes.insert(edges[i].target);
            }
        }
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edgeActive[i]) continue;
            if (!focusNodes.contains(edges[i].source) || !focusNodes.contains(edges[i].target)) {
                edgeActive[i] = false;
            }
        }
    }

    std::vector<int> degree(nodes.size(), 0);
    size_t activeEdgeCount = 0;
    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edgeActive[i]) continue;
        ++activeEdgeCount;
        degree[edges[i].source] += 1;
        degree[edges[i].target] += 1;
    }

    ImGui::Text("Distance Type: %s", graph.value("distanceType", "unknown").c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| Nodes: %zu | Edges: %zu (%zu shown)", nodes.size(), edges.size(), activeEdgeCount);
    ImGui::SliderFloat("Min Similarity", &graphMinSimilarity_, 0.0f, 1.0f, "%.2f");
    ImGui::SliderInt("Top K links per node", &graphTopKPerNode_, 0, 8);
    ImGui::Checkbox("Show Labels", &graphShowLabels_);
    ImGui::SameLine();
    ImGui::Checkbox("Hide Isolated", &graphHideIsolated_);
    ImGui::SameLine();
    ImGui::Checkbox("Focus Selected Node", &graphFocusSelected_);
    if (graphFocusSelected_ && selectedIndex < 0) {
        ImGui::TextDisabled("Select one node to activate focus mode.");
    }

    ImGui::TextDisabled("Dimension Colors:");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendTopEcologicalWorkspace", colorForDimension("ecological"), "Eco");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendTopProductiveWorkspace", colorForDimension("productive"), "Prod");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendTopSocialWorkspace", colorForDimension("social"), "Soc");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendTopMixedWorkspace", colorForDimension("mixed"), "Mixed");

    if (nodes.empty()) {
        ImGui::TextDisabled("No narrative contexts available to render.");
        return;
    }

    ImGui::BeginChild("NarrativeGraphCanvasWorkspace", ImVec2(0.0f, 430.0f), true, ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 80.0f || canvasSize.y < 80.0f) {
        ImGui::EndChild();
        return;
    }
    ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);

    ImGui::InvisibleButton("NarrativeGraphButtonWorkspace", canvasSize, ImGuiButtonFlags_MouseButtonLeft);
    const bool canvasHovered = ImGui::IsItemHovered();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(38, 42, 56, 255), 8.0f);
    drawList->AddRect(canvasMin, canvasMax, IM_COL32(95, 110, 130, 180), 8.0f, 0, 1.0f);

    ImVec2 center(canvasMin.x + canvasSize.x * 0.5f, canvasMin.y + canvasSize.y * 0.5f);
    const float maxRing = std::max(50.0f, std::min(canvasSize.x, canvasSize.y) * 0.42f);
    drawList->AddCircle(center, maxRing, IM_COL32(70, 80, 98, 150), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.65f, IM_COL32(70, 80, 98, 130), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.35f, IM_COL32(70, 80, 98, 110), 120, 1.0f);

    std::vector<int> visibleIndices;
    visibleIndices.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (graphFocusSelected_ && selectedIndex >= 0 && !focusNodes.contains(static_cast<int>(i))) continue;
        if (graphHideIsolated_ && degree[i] == 0) continue;
        nodes[i].dominantDimension = normalizeDimension(nodes[i].dominantDimension);

        if ((nodes[i].dominantDimension == "ecological" && !filterEcological_) ||
            (nodes[i].dominantDimension == "productive" && !filterProductive_) ||
            (nodes[i].dominantDimension == "social" && !filterSocial_) ||
            (nodes[i].dominantDimension == "mixed" && !filterMixed_)) {
            continue;
        }
        visibleIndices.push_back(static_cast<int>(i));
    }

    std::vector<bool> nodeVisible(nodes.size(), false);
    for (int idx : visibleIndices) {
        nodeVisible[idx] = true;
    }

    std::map<std::string, std::vector<int>> buckets;
    for (int idx : visibleIndices) {
        buckets[nodes[idx].dominantDimension].push_back(idx);
    }
    for (auto& [_, bucket] : buckets) {
        std::sort(bucket.begin(), bucket.end(), [&](int a, int b) {
            if (nodes[a].narrativeCount == nodes[b].narrativeCount) return nodes[a].label < nodes[b].label;
            return nodes[a].narrativeCount > nodes[b].narrativeCount;
        });
    }

    const std::map<std::string, float> sectorCenters = {
        {"ecological", -2.20f},
        {"productive", -0.50f},
        {"social", 1.20f},
        {"mixed", 2.50f}
    };
    const float sectorSpan = 1.10f;
    for (const auto& [dim, centerAngle] : sectorCenters) {
        auto it = buckets.find(dim);
        if (it == buckets.end()) continue;
        const auto& bucket = it->second;
        const size_t n = bucket.size();
        for (size_t i = 0; i < n; ++i) {
            const float t = (n <= 1) ? 0.5f : (static_cast<float>(i) / static_cast<float>(n - 1));
            const float angle = centerAngle - sectorSpan * 0.5f + sectorSpan * t;
            const float ring = static_cast<float>(i % 3);
            const float layer = static_cast<float>(i / 3);
            const float radius = maxRing * (0.42f + 0.16f * ring) + 10.0f * layer;
            NarrativeGraphNodeUI& node = nodes[bucket[i]];
            node.position = ImVec2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        }
    }

    int hoveredNode = -1;
    if (canvasHovered) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        for (int idx : visibleIndices) {
            const auto& node = nodes[idx];
            const float dx = mouse.x - node.position.x;
            const float dy = mouse.y - node.position.y;
            if ((dx * dx + dy * dy) <= (node.radius + 4.0f) * (node.radius + 4.0f)) {
                hoveredNode = idx;
                break;
            }
        }
    }

    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edgeActive[i]) continue;
        const auto& edge = edges[i];
        if (!nodeVisible[edge.source] || !nodeVisible[edge.target]) continue;

        const ImVec2 a = nodes[edge.source].position;
        const ImVec2 b = nodes[edge.target].position;
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float length = std::sqrt(dx * dx + dy * dy);
        if (length < 1.0f) continue;

        const float nx = -dy / length;
        const float ny = dx / length;
        const float curvature = std::min(40.0f, length * 0.12f);
        const ImVec2 c1(a.x + dx * 0.33f + nx * curvature, a.y + dy * 0.33f + ny * curvature);
        const ImVec2 c2(a.x + dx * 0.66f + nx * curvature, a.y + dy * 0.66f + ny * curvature);

        const bool selectedEdge = selectedIndex >= 0 && (edge.source == selectedIndex || edge.target == selectedIndex);
        const int alpha = static_cast<int>((selectedEdge ? 120 : 55) + edge.similarity * 120.0f);
        const float thickness = (selectedEdge ? 1.8f : 0.8f) + edge.similarity * 2.4f;
        const ImU32 edgeColor = selectedEdge ? IM_COL32(255, 222, 120, alpha) : IM_COL32(130, 162, 220, alpha);
        drawList->AddBezierCubic(a, c1, c2, b, edgeColor, thickness);
    }

    for (int idx : visibleIndices) {
        auto& node = nodes[idx];
        const bool selected = (selectedGraphNodeId_ == node.id);
        const bool hovered = (hoveredNode == idx);

        const ImU32 baseColor = colorForDimension(node.dominantDimension);
        const float glowRadius = node.radius + (selected ? 7.0f : 4.0f);
        drawList->AddCircleFilled(node.position, glowRadius, IM_COL32(80, 110, 180, selected ? 70 : 30), 36);
        drawList->AddCircleFilled(node.position, node.radius, baseColor, 36);
        drawList->AddCircle(node.position, node.radius, selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(210, 220, 240, hovered ? 255 : 160), 36, selected ? 2.5f : 1.4f);

        if (graphShowLabels_ || hovered || selected) {
            const std::string label = node.label.size() > 36 ? node.label.substr(0, 33) + "..." : node.label;
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f + 1.0f, node.position.y - node.radius - textSize.y - 7.0f + 1.0f), IM_COL32(0, 0, 0, 185), label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f, node.position.y - node.radius - textSize.y - 7.0f), IM_COL32(230, 236, 245, 245), label.c_str());
        }

        if (showNarratives_) {
            const std::string countLabel = std::to_string(node.narrativeCount);
            ImVec2 countSize = ImGui::CalcTextSize(countLabel.c_str());
            drawList->AddText(ImVec2(node.position.x - countSize.x * 0.5f, node.position.y - countSize.y * 0.5f), IM_COL32(20, 24, 35, 245), countLabel.c_str());
        }
    }

    if (canvasHovered && hoveredNode >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        selectedGraphNodeId_ = nodes[hoveredNode].id;
    }
    if (canvasHovered && hoveredNode >= 0) {
        const auto& node = nodes[hoveredNode];
        ImGui::BeginTooltip();
        ImGui::Text("Context: %s", node.label.c_str());
        ImGui::Text("Narratives: %d", node.narrativeCount);
        ImGui::Text("Dimension: %s", labelForDimension(node.dominantDimension));
        ImGui::Text("Intent: %s", node.dominantIntent.c_str());
        if (!node.observationIds.empty()) {
            ImGui::Text("Observation IDs: %zu", node.observationIds.size());
        }
        if (!node.artifactIds.empty()) {
            ImGui::Text("Artifacts: %zu", node.artifactIds.size());
        }
        if (showProcesses_) {
            auto itProfile = contextProfiles_.find(node.id);
            if (itProfile != contextProfiles_.end() && !itProfile->second.processCounts.empty()) {
                ImGui::Separator();
                ImGui::Text("Processos narrados:");
                std::vector<std::pair<std::string, int>> processes(itProfile->second.processCounts.begin(),
                                                                  itProfile->second.processCounts.end());
                std::sort(processes.begin(), processes.end(), [](const auto& a, const auto& b) {
                    if (a.second == b.second) return a.first < b.first;
                    return a.second > b.second;
                });
                int shown = 0;
                for (const auto& [label, count] : processes) {
                    ImGui::BulletText("%s (%d)", label.c_str(), count);
                    if (++shown >= 4) break;
                }
            }
        }
        ImGui::EndTooltip();
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Legend");
    drawDimensionLegendItem("##LegendEcologicalWorkspace", colorForDimension("ecological"), "Ecological");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendProductiveWorkspace", colorForDimension("productive"), "Productive");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendSocialWorkspace", colorForDimension("social"), "Social");
    ImGui::SameLine();
    drawDimensionLegendItem("##LegendMixedWorkspace", colorForDimension("mixed"), "Mixed");
    ImGui::BulletText("Node color: dominant epistemic dimension");
    if (showNarratives_) {
        ImGui::BulletText("Node size: number of narrative observations");
    }
    ImGui::BulletText("Edge width/alpha: narrative similarity (Jaccard)");
    ImGui::BulletText("Edges filtered by Min Similarity and Top-K per node");
    ImGui::BulletText("Focus mode keeps only the selected node and first-hop neighbors");
    ImGui::BulletText("Proximidade narrativa (nao causal)");
    ImGui::BulletText("Sem causalidade");
}

void AnalysisWorkspacePanel::drawContextDetails() {
    if (selectedGraphNodeId_.empty()) {
        ImGui::TextDisabled("Selecione um contexto no grafo para ver detalhes.");
        return;
    }

    auto itProfile = contextProfiles_.find(selectedGraphNodeId_);
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
