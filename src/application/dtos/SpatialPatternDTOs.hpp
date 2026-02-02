#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Application::DTO::SpatialPattern {

struct PatchMetrics {
    double area = 0.0;
    double perimeter = 0.0;
    double axis = 0.0;
    double par = 0.0;
    double shape_index = 0.0;
    double rcc = 0.0;
    double centroidX = 0.0;
    double centroidY = 0.0;
    double centroidZ = 0.0;
};

struct SummaryMetrics {
    int patchCount = 0;
    double meanPar = 0.0;
    double meanShapeIndex = 0.0;
    double meanRcc = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double areaTotal = 0.0;
    double areaMean = 0.0;
    double areaStdDev = 0.0;
    double areaMin = 0.0;
    double areaMax = 0.0;
    double shapeIndexStdDev = 0.0;
    double shapeIndexMin = 0.0;
    double shapeIndexMax = 0.0;
};

struct LabelImage {
    std::vector<uint32_t> labels;
    int width = 0;
    int height = 0;
};

struct GridData {
    std::vector<double> values;
    std::vector<double> elevation;
    int width = 0;
    int height = 0;
    double cellWidth = 1.0;
    double cellHeight = 1.0;
    double originX = 0.0;
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

} // namespace Application::DTO::SpatialPattern
