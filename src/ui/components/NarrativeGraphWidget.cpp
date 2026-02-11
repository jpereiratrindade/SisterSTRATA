#include "ui/components/NarrativeGraphWidget.hpp"
#include "imgui.h"
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <cmath>
#include <string>

namespace {

struct GraphNodeUI {
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

struct GraphEdgeUI {
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

static std::string normalizeDimension(std::string dim) {
    if (dim != "ecological" && dim != "productive" && dim != "social") {
        dim = "mixed";
    }
    return dim;
}

} // namespace

namespace UI::Components {

void NarrativeGraphWidget::draw(const nlohmann::json& graph,
                                 NarrativeGraphState& state,
                                 const char* idPrefix,
                                 const NarrativeGraphOptions& options) {
    ImGui::PushID(idPrefix);

    const auto nodeJson = graph.value("nodes", nlohmann::json::array());
    const auto edgeJson = graph.value("edges", nlohmann::json::array());

    // ── Parse Nodes ──
    std::vector<GraphNodeUI> nodes;
    nodes.reserve(nodeJson.size());
    std::unordered_map<std::string, int> nodeIndex;
    for (const auto& n : nodeJson) {
        if (!n.is_object()) continue;

        GraphNodeUI node;
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

        node.radius = options.scaleRadiusByCount
            ? 11.0f + std::sqrt(static_cast<float>(std::max(1, node.narrativeCount))) * 4.0f
            : 14.0f;

        nodeIndex[node.id] = static_cast<int>(nodes.size());
        nodes.push_back(std::move(node));
    }

    // ── Parse Edges ──
    std::vector<GraphEdgeUI> edges;
    edges.reserve(edgeJson.size());
    for (const auto& e : edgeJson) {
        if (!e.is_object()) continue;
        const std::string sourceId = e.value("source", "");
        const std::string targetId = e.value("target", "");
        auto itA = nodeIndex.find(sourceId);
        auto itB = nodeIndex.find(targetId);
        if (itA == nodeIndex.end() || itB == nodeIndex.end()) continue;

        GraphEdgeUI edge;
        edge.source = itA->second;
        edge.target = itB->second;
        edge.similarity = e.value("similarity", 0.0f);
        edge.distance = e.value("distance", 1.0f);
        edge.sharedTokens = e.value("sharedTokens", 0);
        edges.push_back(edge);
    }

    // ── Validate selection ──
    std::unordered_map<std::string, int> stableNodeLookup;
    stableNodeLookup.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        stableNodeLookup[nodes[i].id] = static_cast<int>(i);
    }
    if (!state.selectedNodeId.empty() && !stableNodeLookup.contains(state.selectedNodeId)) {
        state.selectedNodeId.clear();
    }

    // ── Edge filtering ──
    std::vector<bool> edgeActive(edges.size(), false);
    for (size_t i = 0; i < edges.size(); ++i) {
        edgeActive[i] = edges[i].similarity >= state.minSimilarity;
    }

    const int topK = std::max(0, state.topKPerNode);
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

    // ── Focus mode ──
    int selectedIndex = -1;
    if (!state.selectedNodeId.empty()) {
        auto it = stableNodeLookup.find(state.selectedNodeId);
        if (it != stableNodeLookup.end()) selectedIndex = it->second;
    }

    std::set<int> focusNodes;
    if (state.focusSelected && selectedIndex >= 0) {
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

    // ── Degree and stats ──
    std::vector<int> degree(nodes.size(), 0);
    size_t activeEdgeCount = 0;
    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edgeActive[i]) continue;
        ++activeEdgeCount;
        degree[edges[i].source] += 1;
        degree[edges[i].target] += 1;
    }

    // ── Controls ──
    ImGui::Text("Distance Type: %s", graph.value("distanceType", "unknown").c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("| Nodes: %zu | Edges: %zu (%zu shown)", nodes.size(), edges.size(), activeEdgeCount);
    ImGui::SliderFloat("Min Similarity", &state.minSimilarity, 0.0f, 1.0f, "%.2f");
    ImGui::SliderInt("Top K links per node", &state.topKPerNode, 0, 8);
    ImGui::Checkbox("Show Labels", &state.showLabels);
    ImGui::SameLine();
    ImGui::Checkbox("Hide Isolated", &state.hideIsolated);
    ImGui::SameLine();
    ImGui::Checkbox("Focus Selected Node", &state.focusSelected);
    if (state.focusSelected && selectedIndex < 0) {
        ImGui::TextDisabled("Select one node to activate focus mode.");
    }

    const std::string ecoId = std::string("##LegendTopEco") + idPrefix;
    const std::string prodId = std::string("##LegendTopProd") + idPrefix;
    const std::string socId = std::string("##LegendTopSoc") + idPrefix;
    const std::string mixId = std::string("##LegendTopMix") + idPrefix;
    ImGui::TextDisabled("Dimension Colors:");
    ImGui::SameLine();
    drawDimensionLegendItem(ecoId.c_str(), colorForDimension("ecological"), "Eco");
    ImGui::SameLine();
    drawDimensionLegendItem(prodId.c_str(), colorForDimension("productive"), "Prod");
    ImGui::SameLine();
    drawDimensionLegendItem(socId.c_str(), colorForDimension("social"), "Soc");
    ImGui::SameLine();
    drawDimensionLegendItem(mixId.c_str(), colorForDimension("mixed"), "Mixed");

    if (nodes.empty()) {
        ImGui::TextDisabled("No narrative contexts available to render.");
        ImGui::PopID();
        return;
    }

    // ── Canvas ──
    const std::string canvasChildId = std::string("NarrativeGraphCanvas") + idPrefix;
    const std::string canvasBtnId = std::string("NarrativeGraphButton") + idPrefix;
    ImGui::BeginChild(canvasChildId.c_str(), ImVec2(0.0f, 430.0f), true, ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 80.0f || canvasSize.y < 80.0f) {
        ImGui::EndChild();
        ImGui::PopID();
        return;
    }
    ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);

    ImGui::InvisibleButton(canvasBtnId.c_str(), canvasSize, ImGuiButtonFlags_MouseButtonLeft);
    const bool canvasHovered = ImGui::IsItemHovered();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(38, 42, 56, 255), 8.0f);
    drawList->AddRect(canvasMin, canvasMax, IM_COL32(95, 110, 130, 180), 8.0f, 0, 1.0f);

    ImVec2 center(canvasMin.x + canvasSize.x * 0.5f, canvasMin.y + canvasSize.y * 0.5f);
    const float maxRing = std::max(50.0f, std::min(canvasSize.x, canvasSize.y) * 0.42f);
    drawList->AddCircle(center, maxRing, IM_COL32(70, 80, 98, 150), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.65f, IM_COL32(70, 80, 98, 130), 120, 1.0f);
    drawList->AddCircle(center, maxRing * 0.35f, IM_COL32(70, 80, 98, 110), 120, 1.0f);

    // ── Visibility filter ──
    std::vector<int> visibleIndices;
    visibleIndices.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (state.focusSelected && selectedIndex >= 0 && !focusNodes.contains(static_cast<int>(i))) continue;
        if (state.hideIsolated && degree[i] == 0) continue;
        nodes[i].dominantDimension = normalizeDimension(nodes[i].dominantDimension);

        // Optional dimension filtering
        if (options.filterEcological && nodes[i].dominantDimension == "ecological" && !*options.filterEcological) continue;
        if (options.filterProductive && nodes[i].dominantDimension == "productive" && !*options.filterProductive) continue;
        if (options.filterSocial && nodes[i].dominantDimension == "social" && !*options.filterSocial) continue;
        if (options.filterMixed && nodes[i].dominantDimension == "mixed" && !*options.filterMixed) continue;

        visibleIndices.push_back(static_cast<int>(i));
    }

    std::vector<bool> nodeVisible(nodes.size(), false);
    for (int idx : visibleIndices) {
        nodeVisible[idx] = true;
    }

    // ── Sector layout ──
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
            GraphNodeUI& node = nodes[bucket[i]];
            node.position = ImVec2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        }
    }

    // ── Hover detection ──
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

    // ── Draw edges ──
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

    // ── Draw nodes ──
    for (int idx : visibleIndices) {
        auto& node = nodes[idx];
        const bool selected = (state.selectedNodeId == node.id);
        const bool hovered = (hoveredNode == idx);

        const ImU32 baseColor = colorForDimension(node.dominantDimension);
        const float glowRadius = node.radius + (selected ? 7.0f : 4.0f);
        drawList->AddCircleFilled(node.position, glowRadius, IM_COL32(80, 110, 180, selected ? 70 : 30), 36);
        drawList->AddCircleFilled(node.position, node.radius, baseColor, 36);
        drawList->AddCircle(node.position, node.radius, selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(210, 220, 240, hovered ? 255 : 160), 36, selected ? 2.5f : 1.4f);

        if (state.showLabels || hovered || selected) {
            const std::string label = node.label.size() > 36 ? node.label.substr(0, 33) + "..." : node.label;
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f + 1.0f, node.position.y - node.radius - textSize.y - 7.0f + 1.0f), IM_COL32(0, 0, 0, 185), label.c_str());
            drawList->AddText(ImVec2(node.position.x - textSize.x * 0.5f, node.position.y - node.radius - textSize.y - 7.0f), IM_COL32(230, 236, 245, 245), label.c_str());
        }

        if (options.showNarrativeCounts) {
            const std::string countLabel = std::to_string(node.narrativeCount);
            ImVec2 countSize = ImGui::CalcTextSize(countLabel.c_str());
            drawList->AddText(ImVec2(node.position.x - countSize.x * 0.5f, node.position.y - countSize.y * 0.5f), IM_COL32(20, 24, 35, 245), countLabel.c_str());
        }
    }

    // ── Click selection ──
    if (canvasHovered && hoveredNode >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        state.selectedNodeId = nodes[hoveredNode].id;
    }

    // ── Tooltip ──
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
        if (!node.topTokens.empty()) {
            std::string top;
            for (size_t i = 0; i < node.topTokens.size(); ++i) {
                if (i > 0) top += ", ";
                top += node.topTokens[i];
                if (top.size() > 170) {
                    top += "...";
                    break;
                }
            }
            ImGui::TextWrapped("Top Tokens: %s", top.c_str());
        }
        // Optional process tooltip
        if (options.showProcesses && options.processProfiles) {
            auto itProfile = options.processProfiles->find(node.id);
            if (itProfile != options.processProfiles->end() && !itProfile->second.empty()) {
                ImGui::Separator();
                ImGui::Text("Processos narrados:");
                std::vector<std::pair<std::string, int>> processes(itProfile->second.begin(),
                                                                   itProfile->second.end());
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

    // ── Legend ──
    const std::string ecoLegId = std::string("##LegendEco") + idPrefix;
    const std::string prodLegId = std::string("##LegendProd") + idPrefix;
    const std::string socLegId = std::string("##LegendSoc") + idPrefix;
    const std::string mixLegId = std::string("##LegendMix") + idPrefix;
    ImGui::Separator();
    ImGui::Text("Legend");
    drawDimensionLegendItem(ecoLegId.c_str(), colorForDimension("ecological"), "Ecological");
    ImGui::SameLine();
    drawDimensionLegendItem(prodLegId.c_str(), colorForDimension("productive"), "Productive");
    ImGui::SameLine();
    drawDimensionLegendItem(socLegId.c_str(), colorForDimension("social"), "Social");
    ImGui::SameLine();
    drawDimensionLegendItem(mixLegId.c_str(), colorForDimension("mixed"), "Mixed");
    ImGui::BulletText("Node color: dominant epistemic dimension");
    if (options.showNarrativeCounts) {
        ImGui::BulletText("Node size: number of narrative observations");
    }
    ImGui::BulletText("Edge width/alpha: narrative similarity (Jaccard)");
    ImGui::BulletText("Edges filtered by Min Similarity and Top-K per node");
    ImGui::BulletText("Focus mode keeps only the selected node and first-hop neighbors");

    // ── Selected node details ──
    if (!state.selectedNodeId.empty()) {
        auto itSelected = std::find_if(nodes.begin(), nodes.end(), [&](const auto& item) {
            return item.id == state.selectedNodeId;
        });
        if (itSelected != nodes.end()) {
            ImGui::Separator();
            ImGui::Text("Selected Context: %s", itSelected->label.c_str());
            ImGui::TextDisabled("Dominant Dimension: %s", labelForDimension(itSelected->dominantDimension));
            ImGui::TextDisabled("Dominant Intent: %s", itSelected->dominantIntent.c_str());
            ImGui::TextDisabled("Narrative Count: %d", itSelected->narrativeCount);

            if (!itSelected->observationIds.empty()) {
                ImGui::Text("Observation IDs");
                const std::string obsChildId = std::string("GraphObsIds") + idPrefix;
                ImGui::BeginChild(obsChildId.c_str(), ImVec2(0.0f, 74.0f), true);
                for (const auto& id : itSelected->observationIds) {
                    ImGui::BulletText("%s", id.c_str());
                }
                ImGui::EndChild();
            }

            if (!itSelected->artifactIds.empty()) {
                ImGui::Text("IW Artifacts");
                const std::string artChildId = std::string("GraphArtIds") + idPrefix;
                ImGui::BeginChild(artChildId.c_str(), ImVec2(0.0f, 64.0f), true);
                for (const auto& artifactId : itSelected->artifactIds) {
                    ImGui::BulletText("%s", artifactId.c_str());
                }
                ImGui::EndChild();
            }
        }
    }

    ImGui::PopID();
}

} // namespace UI::Components
