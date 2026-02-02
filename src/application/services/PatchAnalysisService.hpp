#pragma once

#include "application/dtos/SpatialPatternDTOs.hpp"
#include "core/domain/spatial_pattern/PatchAnalysis.hpp"

namespace Application::Services {

class PatchAnalysisService {
public:
    static Application::DTO::SpatialPattern::GridData loadGridCsv(const std::string& path) {
        auto grid = Core::Domain::SpatialPattern::LoadGridCsv(path);
        return toDTO(grid);
    }

    static Application::DTO::SpatialPattern::AnalysisResult analyzeGrid(
        const Application::DTO::SpatialPattern::GridData& grid,
        const Application::DTO::SpatialPattern::AnalysisConfig& cfg
    ) {
        auto coreGrid = toCore(grid);
        Core::Domain::SpatialPattern::AnalysisConfig coreCfg;
        coreCfg.threshold = cfg.threshold;
        coreCfg.byClass = cfg.byClass;
        coreCfg.keepLabels = cfg.keepLabels;
        auto res = Core::Domain::SpatialPattern::AnalyzeGrid(coreGrid, coreCfg);
        return toDTO(res);
    }

    static void writeCsv(const std::string& outPath,
                         const std::vector<Application::DTO::SpatialPattern::PatchMetrics>& patches,
                         const Application::DTO::SpatialPattern::SummaryMetrics& summary,
                         bool summaryOnly) {
        Core::Domain::SpatialPattern::SummaryMetrics coreSummary = toCore(summary);
        std::vector<Core::Domain::SpatialPattern::PatchMetrics> corePatches;
        corePatches.reserve(patches.size());
        for (const auto& p : patches) corePatches.push_back(toCore(p));
        Core::Domain::SpatialPattern::WriteCsv(outPath, corePatches, coreSummary, summaryOnly);
    }

    static void writeLabelCsv(const std::string& outPath,
                              const Application::DTO::SpatialPattern::LabelImage& labels) {
        Core::Domain::SpatialPattern::LabelImage coreLabels;
        coreLabels.labels = labels.labels;
        coreLabels.width = labels.width;
        coreLabels.height = labels.height;
        Core::Domain::SpatialPattern::WriteLabelCsv(outPath, coreLabels);
    }

private:
    static Application::DTO::SpatialPattern::GridData toDTO(const Core::Domain::SpatialPattern::GridData& grid) {
        Application::DTO::SpatialPattern::GridData dto;
        dto.values = grid.values;
        dto.elevation = grid.elevation;
        dto.width = grid.width;
        dto.height = grid.height;
        dto.cellWidth = grid.cellWidth;
        dto.cellHeight = grid.cellHeight;
        dto.originX = grid.originX;
        dto.originY = grid.originY;
        dto.hasNoData = grid.hasNoData;
        dto.noDataValue = grid.noDataValue;
        dto.noDataIsNaN = grid.noDataIsNaN;
        return dto;
    }

    static Core::Domain::SpatialPattern::GridData toCore(const Application::DTO::SpatialPattern::GridData& grid) {
        Core::Domain::SpatialPattern::GridData core;
        core.values = grid.values;
        core.elevation = grid.elevation;
        core.width = grid.width;
        core.height = grid.height;
        core.cellWidth = grid.cellWidth;
        core.cellHeight = grid.cellHeight;
        core.originX = grid.originX;
        core.originY = grid.originY;
        core.hasNoData = grid.hasNoData;
        core.noDataValue = grid.noDataValue;
        core.noDataIsNaN = grid.noDataIsNaN;
        return core;
    }

    static Application::DTO::SpatialPattern::AnalysisResult toDTO(const Core::Domain::SpatialPattern::AnalysisResult& res) {
        Application::DTO::SpatialPattern::AnalysisResult dto;
        dto.patches.reserve(res.patches.size());
        for (const auto& p : res.patches) dto.patches.push_back(toDTO(p));
        dto.summary = toDTO(res.summary);
        dto.labelImage.labels = res.labelImage.labels;
        dto.labelImage.width = res.labelImage.width;
        dto.labelImage.height = res.labelImage.height;
        return dto;
    }

    static Application::DTO::SpatialPattern::PatchMetrics toDTO(const Core::Domain::SpatialPattern::PatchMetrics& p) {
        Application::DTO::SpatialPattern::PatchMetrics dto;
        dto.area = p.area;
        dto.perimeter = p.perimeter;
        dto.axis = p.axis;
        dto.par = p.par;
        dto.shape_index = p.shape_index;
        dto.rcc = p.rcc;
        dto.centroidX = p.centroidX;
        dto.centroidY = p.centroidY;
        dto.centroidZ = p.centroidZ;
        return dto;
    }

    static Application::DTO::SpatialPattern::SummaryMetrics toDTO(const Core::Domain::SpatialPattern::SummaryMetrics& s) {
        Application::DTO::SpatialPattern::SummaryMetrics dto;
        dto.patchCount = s.patchCount;
        dto.meanPar = s.meanPar;
        dto.meanShapeIndex = s.meanShapeIndex;
        dto.meanRcc = s.meanRcc;
        dto.s1 = s.s1;
        dto.s2 = s.s2;
        dto.areaTotal = s.areaTotal;
        dto.areaMean = s.areaMean;
        dto.areaStdDev = s.areaStdDev;
        dto.areaMin = s.areaMin;
        dto.areaMax = s.areaMax;
        dto.shapeIndexStdDev = s.shapeIndexStdDev;
        dto.shapeIndexMin = s.shapeIndexMin;
        dto.shapeIndexMax = s.shapeIndexMax;
        return dto;
    }

    static Core::Domain::SpatialPattern::PatchMetrics toCore(const Application::DTO::SpatialPattern::PatchMetrics& p) {
        Core::Domain::SpatialPattern::PatchMetrics core;
        core.area = p.area;
        core.perimeter = p.perimeter;
        core.axis = p.axis;
        core.par = p.par;
        core.shape_index = p.shape_index;
        core.rcc = p.rcc;
        core.centroidX = p.centroidX;
        core.centroidY = p.centroidY;
        core.centroidZ = p.centroidZ;
        return core;
    }

    static Core::Domain::SpatialPattern::SummaryMetrics toCore(const Application::DTO::SpatialPattern::SummaryMetrics& s) {
        Core::Domain::SpatialPattern::SummaryMetrics core;
        core.patchCount = s.patchCount;
        core.meanPar = s.meanPar;
        core.meanShapeIndex = s.meanShapeIndex;
        core.meanRcc = s.meanRcc;
        core.s1 = s.s1;
        core.s2 = s.s2;
        core.areaTotal = s.areaTotal;
        core.areaMean = s.areaMean;
        core.areaStdDev = s.areaStdDev;
        core.areaMin = s.areaMin;
        core.areaMax = s.areaMax;
        core.shapeIndexStdDev = s.shapeIndexStdDev;
        core.shapeIndexMin = s.shapeIndexMin;
        core.shapeIndexMax = s.shapeIndexMax;
        return core;
    }
};

} // namespace Application::Services
