#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Core::Domain::Analysis {

struct PatchMetrics {
    double area = 0.0;
    double perimeter = 0.0;
    double axis = 0.0;
    double par = 0.0;          // Perimeter-Area Ratio
    double shape_index = 0.0;  // Patton's Shape Index
    double rcc = 0.0;
};

struct SummaryMetrics {
    int patchCount = 0;
    double meanPar = 0.0;
    double meanShapeIndex = 0.0;
    double meanRcc = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
};

struct LabelImage {
    std::vector<uint32_t> labels;
    int width = 0;
    int height = 0;
};

struct GridData {
    std::vector<double> values;
    std::vector<double> elevation; // New: Stores Z height for 3D overlay
    int width = 0;
    int height = 0;
    double cellWidth = 1.0;
    double cellHeight = 1.0;
    double originX = 0.0; // World coordinate of grid (0,0)
    double originY = 0.0;
    bool hasNoData = false;
    double noDataValue = 0.0;
    bool noDataIsNaN = false;
};

struct AnalysisConfig {
    double threshold = 0.0;
    bool byClass = false;
    bool keepLabels = false;
};

struct AnalysisResult {
    std::vector<PatchMetrics> patches;
    SummaryMetrics summary;
    LabelImage labelImage;
};

AnalysisResult AnalyzeGrid(const GridData& grid, const AnalysisConfig& cfg);
GridData LoadGridCsv(const std::string& path);
void WriteCsv(const std::string& outPath, const std::vector<PatchMetrics>& patches, const SummaryMetrics& summary, bool summaryOnly);
void WriteLabelCsv(const std::string& outPath, const LabelImage& labels);

} // namespace Core::Domain::Analysis
