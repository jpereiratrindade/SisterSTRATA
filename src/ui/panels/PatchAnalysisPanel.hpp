#pragma once

#include <array>
#include <string>
#include <vector>
#include "imgui.h"
#include <glm/glm.hpp>
#include "core/domain/analysis/PatchAnalysis.hpp"
#include "ui/components/FileSelector.hpp"

namespace UI::Panels {

struct LegendEntry {
    int id = 0;
    std::string name;
    ImVec4 color{};
};

class PatchAnalysisPanel {
public:
    void draw(bool* open);
    void SetInputPath(const std::string& path);
    void SetLegendPath(const std::string& path);

private:
    void clearResults();
    struct PatchState {
        std::array<char, 256> inputPath{"assets/data/patch_grid.csv"};
        std::array<char, 256> outputCsvPath{"assets/data/patches.csv"};
        std::array<char, 256> outputLabelsPath{"assets/data/patch_labels.csv"};
        std::array<char, 256> legendPath{"assets/data/sibcs_legend.csv"};
        std::array<char, 256> snapshotPath{"assets/data/patch_snapshot.png"};
        bool byClass = false;
        bool summaryOnly = false;
        bool exportCsv = true;
        bool exportLabels = false;
        double threshold = 0.0;
        double cellWidth = 1.0;
        double cellHeight = 1.0;
        bool showPreview = true;
        float previewCellSize = 8.0f;
        int paletteMode = 0;
        int shapeMode = 0;
        bool useLabelColors = true;
        bool useValueAsHeight = false;
        float heightScale = 1.0f;
        float pointSize = 6.0f;
        bool useLegendColors = false;
        bool legendLoaded = false;
        
        // Manual Offset for Debugging/Fine-tuning
        glm::vec3 manualOffset = glm::vec3(0.0f);
        bool useManualOffset = false;

        // File Selection
        UI::Components::FileSelector fileSelector{"Select Grid CSV"};
        bool showFileSelector = false;

        Core::Domain::Analysis::SummaryMetrics summary;
        std::string status;
        std::string error;
        bool lastRunSuccess = false;
        Core::Domain::Analysis::LabelImage lastLabels;
        std::vector<LegendEntry> legendEntries;
    };

    PatchState state_;
};

} // namespace UI::Panels
