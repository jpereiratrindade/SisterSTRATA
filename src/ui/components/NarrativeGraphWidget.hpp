#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <functional>

/**
 * @brief Shared, reusable widget for rendering the Narrative Context Graph.
 *
 * Used by both NarrativePanel and AnalysisWorkspacePanel to avoid code
 * duplication (~370 lines of identical rendering logic).
 *
 * DESIGN:
 * - Stateless renderer: all mutable state lives in NarrativeGraphState.
 * - Callers pass a const JSON graph and their own state struct.
 * - Optional callbacks for dimension filtering and process tooltips.
 */
namespace UI::Components {

/// Per-instance mutable state for a NarrativeGraphWidget.
struct NarrativeGraphState {
    float minSimilarity = 0.35f;
    int topKPerNode = 3;
    bool showLabels = false;
    bool hideIsolated = false;
    bool focusSelected = false;
    std::string selectedNodeId;
};

/// Optional configuration to enable advanced features (filters, etc).
struct NarrativeGraphOptions {
    /// If non-null, used as dimension visibility filters (eco/prod/soc/mixed).
    bool* filterEcological = nullptr;
    bool* filterProductive = nullptr;
    bool* filterSocial = nullptr;
    bool* filterMixed = nullptr;

    /// If true, show narrative count inside nodes.
    bool showNarrativeCounts = true;

    /// If true, scale node radius by narrative count; otherwise fixed 14.
    bool scaleRadiusByCount = true;

    /// Optional: process counts per context ID for tooltip enrichment.
    using ProcessMap = std::unordered_map<std::string,
        std::unordered_map<std::string, int>>;
    const ProcessMap* processProfiles = nullptr;
    bool showProcesses = false;
};

class NarrativeGraphWidget {
public:
    /**
     * @brief Draw the narrative context graph.
     * @param graph JSON object with "nodes" and "edges" arrays.
     * @param state Mutable state (persists across frames).
     * @param idPrefix ImGui ID prefix to avoid collisions between instances.
     * @param options Advanced options (filters, process tooltips). Defaults to basic mode.
     */
    static void draw(const nlohmann::json& graph,
                     NarrativeGraphState& state,
                     const char* idPrefix = "NarrativeGraph",
                     const NarrativeGraphOptions& options = {});
};

} // namespace UI::Components
