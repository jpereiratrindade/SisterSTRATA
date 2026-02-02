#include "TimelinePanel.hpp"
#include "application/services/FourthDimensionService.hpp"
#include "application/services/PatchAnalysisService.hpp"
#include "application/services/World3DService.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

namespace UI::Panels {

void TimelinePanel::applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice) {
    if (slice.isProxy()) {
        Application::Services::FourthDimensionService::loadSliceFromDisk(
            const_cast<Core::Domain::FourthDimension::TimeSlice&>(slice)
        );
    }

    Application::Services::World3DService::applyClassificationVisualization(slice.getEcologicalCoverState());

    const auto& cover = slice.getEcologicalCoverState();
    if (!cover.empty()) {
        Application::DTO::SpatialPattern::GridData grid;
        grid.values.assign(cover.begin(), cover.end());
        grid.width = static_cast<int>(std::sqrt(grid.values.size()));
        grid.height = grid.width;
        Application::DTO::SpatialPattern::AnalysisConfig cfg;
        cfg.threshold = 0.0;
        cfg.byClass = true;
        cfg.keepLabels = true;
        lastPatchAnalysis_ = Application::Services::PatchAnalysisService::analyzeGrid(grid, cfg);
    }
}

void TimelinePanel::saveAnalysisToFile(const std::string& content, const std::string& type) {
    std::string filename = "hermeneutic_" + type + "_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "SISTERSTRATA HERMENEUTIC ANALYSIS (" << type << ")\n";
        file << "==========================================\n\n";
        file << content << "\n";
        file.close();
    }
}

std::string TimelinePanel::getClassDistribution(const Core::Domain::FourthDimension::TimeSlice& slice) {
    auto& mutableSlice = const_cast<Core::Domain::FourthDimension::TimeSlice&>(slice);
    if (mutableSlice.isProxy()) {
        Application::Services::FourthDimensionService::loadSliceFromDisk(mutableSlice);
    }

    const auto& cover = slice.getEcologicalCoverState();
    if (cover.empty()) return "Nenhum dado capturado";

    std::map<int, size_t> counts;
    for (int code : cover) {
        if (code != -1) counts[code]++;
    }

    float gridSpacing = 2.0f;
    if (vegPanel_) {
        const auto& vertices = Application::Services::World3DService::getVertices();
        if (vertices.size() > 1) {
            float d = std::abs(vertices[1].pos.x - vertices[0].pos.x);
            if (d > 0.001f) gridSpacing = d;
        }
    }

    int w = static_cast<int>(std::sqrt(cover.size()));
    int h = w;

    std::stringstream ss;
    ss << "[" << slice.getMetadata() << "]: ";
    bool first = true;

    auto type = slice.getClassificationType();
    if (slice.getMetadata().find("Semantic") != std::string::npos) {
        type = Core::Domain::FourthDimension::ClassificationType::SemanticCode;
    }

    if (type == Core::Domain::FourthDimension::ClassificationType::ScenarioIndex) {
        if (vegPanel_) {
            const auto& scenarios = vegPanel_->getScenarioDTOs();
            for (size_t i = 0; i < scenarios.size(); ++i) {
                if (!first) ss << ", ";
                int code = static_cast<int>(i);
                size_t count = counts.count(code) ? counts[code] : 0;
                float pct = 0.0f;
                if (!cover.empty()) pct = (static_cast<float>(count) / cover.size()) * 100.0f;
                float areaHa = (count * gridSpacing * gridSpacing) / 10000.0f;

                ss << "Scenario " << scenarios[i].id << ": " << std::fixed << std::setprecision(1) << pct << "%";
                if (count > 0 && w * h == static_cast<int>(cover.size())) {
                    Application::DTO::SpatialPattern::GridData grid;
                    grid.width = w;
                    grid.height = h;
                    grid.cellWidth = gridSpacing;
                    grid.cellHeight = gridSpacing;
                    grid.values.assign(cover.size(), 0.0);
                    for (size_t j = 0; j < cover.size(); ++j) if (cover[j] == code) grid.values[j] = 1.0;
                    Application::DTO::SpatialPattern::AnalysisConfig cfg;
                    cfg.threshold = 0.5;
                    cfg.byClass = false;
                    cfg.keepLabels = false;
                    auto res = Application::Services::PatchAnalysisService::analyzeGrid(grid, cfg);
                    ss << " (" << areaHa << "ha, " << res.summary.patchCount << " patches, SI: "
                       << static_cast<float>(res.summary.meanShapeIndex) << ")";
                } else if (count > 0) {
                    ss << " (" << areaHa << "ha)";
                }
                first = false;
            }
        }
    } else {
        for (const auto& entry : counts) {
            if (!first) ss << ", ";
            int code = entry.first;
            size_t count = entry.second;
            float pct = (static_cast<float>(count) / cover.size()) * 100.0f;
            float areaHa = (count * gridSpacing * gridSpacing) / 10000.0f;

            ss << "Class " << code << ": " << std::fixed << std::setprecision(1) << pct << "%";
            if (w * h == static_cast<int>(cover.size())) {
                Application::DTO::SpatialPattern::GridData grid;
                grid.width = w;
                grid.height = h;
                grid.cellWidth = gridSpacing;
                grid.cellHeight = gridSpacing;
                grid.values.assign(cover.size(), 0.0);
                for (size_t j = 0; j < cover.size(); ++j) if (cover[j] == code) grid.values[j] = 1.0;
                Application::DTO::SpatialPattern::AnalysisConfig cfg;
                cfg.threshold = 0.5;
                cfg.byClass = false;
                cfg.keepLabels = false;
                auto res = Application::Services::PatchAnalysisService::analyzeGrid(grid, cfg);
                ss << " (" << areaHa << "ha, " << res.summary.patchCount << " patches, SI: "
                   << static_cast<float>(res.summary.meanShapeIndex) << ")";
            } else {
                ss << " (" << areaHa << "ha)";
            }
            first = false;
        }
    }

    return ss.str();
}

} // namespace UI::Panels
