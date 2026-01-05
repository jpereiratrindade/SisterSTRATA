#include "PatchAnalysis.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <cctype>

namespace Core::Domain::SpatialPattern {

namespace {

struct ComponentAccum {
    std::uint64_t count = 0;
    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    double perimeter = 0.0;
    
    // Centroid sums
    double sumX = 0.0;
    double sumY = 0.0;
    double sumZ = 0.0;
};

bool IsNoData(const GridData& data, double value) {
    if (!data.hasNoData) return false;
    if (data.noDataIsNaN) {
        return std::isnan(value);
    }
    return value == data.noDataValue;
}

ComponentAccum FloodFillBinary(const GridData& data,
                               int startX,
                               int startY,
                               double threshold,
                               const std::vector<double>& buffer,
                               std::vector<uint8_t>& visited,
                               std::vector<uint32_t>* labels,
                               uint32_t labelId) {
    auto idx = [&](int x, int y) { return y * data.width + x; };
    auto isValid = [&](int x, int y) {
        double val = buffer[idx(x, y)];
        if (IsNoData(data, val)) return false;
        return val > threshold;
    };

    ComponentAccum acc{};
    acc.minX = acc.maxX = startX;
    acc.minY = acc.maxY = startY;

    std::queue<int> q;
    q.push(idx(startX, startY));
    visited[idx(startX, startY)] = 1;
    if (labels) (*labels)[idx(startX, startY)] = labelId;

    const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        int cx = current % data.width;
        int cy = current / data.width;
        ++acc.count;
        acc.sumX += cx;
        acc.sumY += cy;
        if (!data.elevation.empty()) {
            acc.sumZ += data.elevation[current];
        }

        acc.minX = std::min(acc.minX, cx);
        acc.maxX = std::max(acc.maxX, cx);
        acc.minY = std::min(acc.minY, cy);
        acc.maxY = std::max(acc.maxY, cy);

        for (const auto& d : dirs) {
            int nx = cx + d[0];
            int ny = cy + d[1];
            if (nx >= 0 && nx < data.width && ny >= 0 && ny < data.height) {
                int npos = idx(nx, ny);
                if (isValid(nx, ny)) {
                    if (!visited[npos]) {
                        visited[npos] = 1;
                        if (labels) (*labels)[npos] = labelId;
                        q.push(npos);
                    }
                } else {
                    acc.perimeter += (d[0] != 0) ? data.cellHeight : data.cellWidth;
                }
            } else {
                acc.perimeter += (d[0] != 0) ? data.cellHeight : data.cellWidth;
            }
        }
    }
    return acc;
}

ComponentAccum FloodFillByClass(const GridData& data,
                                int startX,
                                int startY,
                                const std::vector<double>& buffer,
                                std::vector<uint8_t>& visited,
                                std::vector<uint32_t>* labels,
                                uint32_t labelId) {
    auto idx = [&](int x, int y) { return y * data.width + x; };
    int32_t baseClass = static_cast<int32_t>(buffer[idx(startX, startY)]);
    ComponentAccum acc{};
    acc.minX = acc.maxX = startX;
    acc.minY = acc.maxY = startY;

    std::queue<int> q;
    q.push(idx(startX, startY));
    visited[idx(startX, startY)] = 1;
    if (labels) (*labels)[idx(startX, startY)] = labelId;

    const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        int cx = current % data.width;
        int cy = current / data.width;
        ++acc.count;
        acc.sumX += cx;
        acc.sumY += cy;
        if (!data.elevation.empty()) {
            acc.sumZ += data.elevation[current];
        }

        acc.minX = std::min(acc.minX, cx);
        acc.maxX = std::max(acc.maxX, cx);
        acc.minY = std::min(acc.minY, cy);
        acc.maxY = std::max(acc.maxY, cy);

        for (const auto& d : dirs) {
            int nx = cx + d[0];
            int ny = cy + d[1];
            if (nx >= 0 && nx < data.width && ny >= 0 && ny < data.height) {
                int npos = idx(nx, ny);
                int32_t val = static_cast<int32_t>(buffer[npos]);
                if (val == baseClass) {
                    if (!visited[npos]) {
                        visited[npos] = 1;
                        if (labels) (*labels)[npos] = labelId;
                        q.push(npos);
                    }
                } else {
                    acc.perimeter += (d[0] != 0) ? data.cellHeight : data.cellWidth;
                }
            } else {
                acc.perimeter += (d[0] != 0) ? data.cellHeight : data.cellWidth;
            }
        }
    }
    return acc;
}

LabelImage LabelComponents(const GridData& data, const AnalysisConfig& cfg, std::vector<PatchMetrics>& patches) {
    auto idx = [&](int x, int y) { return y * data.width + x; };
    std::vector<uint8_t> visited(static_cast<size_t>(data.width) * data.height, 0);

    LabelImage labelImg{};
    labelImg.width = data.width;
    labelImg.height = data.height;
    if (cfg.keepLabels) {
        labelImg.labels.assign(static_cast<size_t>(data.width) * data.height, 0u);
    }

    for (int y = 0; y < data.height; ++y) {
        for (int x = 0; x < data.width; ++x) {
            int pos = idx(x, y);
            if (visited[pos]) continue;
            double val = data.values[pos];
            if (IsNoData(data, val)) continue;
            if (!cfg.byClass && !(val > cfg.threshold)) continue;
            if (cfg.byClass && static_cast<int32_t>(val) == 0) continue;

            uint32_t labelId = cfg.keepLabels ? static_cast<uint32_t>(patches.size() + 1) : 0u;
            ComponentAccum acc = cfg.byClass
                                     ? FloodFillByClass(data, x, y, data.values, visited, cfg.keepLabels ? &labelImg.labels : nullptr, labelId)
                                     : FloodFillBinary(data, x, y, cfg.threshold, data.values, visited, cfg.keepLabels ? &labelImg.labels : nullptr, labelId);

            PatchMetrics pm{};
            pm.area = static_cast<double>(acc.count) * data.cellWidth * data.cellHeight;
            double widthM = (acc.maxX - acc.minX + 1) * data.cellWidth;
            double heightM = (acc.maxY - acc.minY + 1) * data.cellHeight;
            pm.axis = std::max(widthM, heightM);
            pm.perimeter = acc.perimeter;
            
            if (acc.count > 0) {
                pm.centroidX = acc.sumX / static_cast<double>(acc.count);
                pm.centroidY = acc.sumY / static_cast<double>(acc.count);
                pm.centroidZ = acc.sumZ / static_cast<double>(acc.count);
            }

            patches.push_back(pm);
        }
    }

    if (!cfg.keepLabels) labelImg.labels.clear();
    return labelImg;
}

SummaryMetrics ComputeSummary(std::vector<PatchMetrics>& patches) {
    SummaryMetrics summary{};
    summary.patchCount = static_cast<int>(patches.size());
    if (patches.empty()) return summary;

    double sumPar = 0.0;
    double sumShapeIndex = 0.0;
    double sumRcc = 0.0;
    double sumP4OverSqrt = 0.0;

    for (auto& p : patches) {
        if (p.area <= 0.0) continue;
        p.par = p.perimeter / p.area;
        p.shape_index = (0.282 * p.perimeter) / std::sqrt(p.area);
        p.rcc = (2.0 * std::sqrt(p.area / 3.14)) / (p.axis > 0.0 ? p.axis : 1.0);

        sumPar += p.par;
        sumShapeIndex += p.shape_index;
        sumRcc += p.rcc;
        sumP4OverSqrt += (p.perimeter / 4.0) / std::sqrt(p.area);
    }

    double n = static_cast<double>(summary.patchCount);
    summary.meanPar = sumPar / n;
    summary.meanShapeIndex = sumShapeIndex / n;
    summary.meanRcc = sumRcc / n;
    summary.s1 = (sumPar == 0.0) ? 0.0 : 1.0 / (n * sumPar);
    summary.s2 = (sumP4OverSqrt == 0.0) ? 0.0 : 1.0 / (n * sumP4OverSqrt);
    return summary;
}

} // namespace

GridData LoadGridCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open CSV grid: " + path);
    }

    GridData grid{};
    std::string line;
    int lineNo = 0;
    int width = 0;

    while (std::getline(file, line)) {
        ++lineNo;
        // Check for Metadata in comments
        if (line.rfind("# Origin:", 0) == 0) { // Starts with
             std::stringstream meta(line.substr(9));
             char comma;
             meta >> grid.originX >> comma >> grid.originY;
        }

        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        bool hasNonSpace = false;
        for (char& c : line) {
            if (c == ',' || c == '\t') {
                c = ' ';
            }
            if (!std::isspace(static_cast<unsigned char>(c))) {
                hasNonSpace = true;
            }
        }
        if (!hasNonSpace) continue;

        std::stringstream ss(line);
        std::vector<double> row;
        double value = 0.0;
        while (ss >> value) {
            row.push_back(value);
            if (std::isnan(value)) {
                grid.hasNoData = true;
                grid.noDataIsNaN = true;
            }
        }

        if (row.empty()) continue;
        if (width == 0) {
            width = static_cast<int>(row.size());
        } else if (static_cast<int>(row.size()) != width) {
            throw std::runtime_error("Inconsistent row width at line " + std::to_string(lineNo));
        }

        grid.values.insert(grid.values.end(), row.begin(), row.end());
        ++grid.height;
    }

    if (width <= 0 || grid.height <= 0) {
        throw std::runtime_error("CSV grid is empty: " + path);
    }

    grid.width = width;
    return grid;
}

AnalysisResult AnalyzeGrid(const GridData& grid, const AnalysisConfig& cfg) {
    if (grid.width <= 0 || grid.height <= 0) {
        throw std::runtime_error("Grid dimensions must be positive.");
    }
    if (grid.values.size() != static_cast<size_t>(grid.width) * grid.height) {
        throw std::runtime_error("Grid values size does not match width/height.");
    }

    std::vector<PatchMetrics> patches;
    LabelImage labels = LabelComponents(grid, cfg, patches);
    SummaryMetrics summary = ComputeSummary(patches);

    AnalysisResult result{};
    result.patches = std::move(patches);
    result.summary = summary;
    result.labelImage = std::move(labels);
    return result;
}

void WriteCsv(const std::string& outPath,
              const std::vector<PatchMetrics>& patches,
              const SummaryMetrics& summary,
              bool summaryOnly) {
    std::ofstream csv(outPath);
    if (!csv) {
        throw std::runtime_error("Unable to open output CSV: " + outPath);
    }
    csv.setf(std::ios::fixed);
    csv.precision(6);

    if (summaryOnly) {
        csv << "patches,mean_par,mean_shape_index,mean_rcc,s1,s2\n";
        csv << summary.patchCount << "," << summary.meanPar << "," << summary.meanShapeIndex << ","
            << summary.meanRcc << "," << summary.s1 << "," << summary.s2 << "\n";
        return;
    }

    csv << "id,area,perimeter,axis,par,shape_index,rcc\n";
    int idx = 1;
    for (const auto& p : patches) {
        csv << idx++ << "," << p.area << "," << p.perimeter << ","
            << p.axis << "," << p.par << "," << p.shape_index << "," << p.rcc << "\n";
    }
    csv << "\n";
    csv << "patches,mean_par,mean_shape_index,mean_rcc,s1,s2\n";
    csv << summary.patchCount << "," << summary.meanPar << "," << summary.meanShapeIndex << ","
        << summary.meanRcc << "," << summary.s1 << "," << summary.s2 << "\n";
}

void WriteLabelCsv(const std::string& outPath, const LabelImage& labels) {
    if (labels.labels.empty()) {
        throw std::runtime_error("Empty label map: nothing to write.");
    }
    if (labels.width <= 0 || labels.height <= 0 ||
        static_cast<size_t>(labels.width) * labels.height != labels.labels.size()) {
        throw std::runtime_error("Label map dimensions do not match label buffer.");
    }

    std::ofstream csv(outPath);
    if (!csv) {
        throw std::runtime_error("Unable to open label CSV: " + outPath);
    }

    for (int y = 0; y < labels.height; ++y) {
        for (int x = 0; x < labels.width; ++x) {
            size_t idx = static_cast<size_t>(y * labels.width + x);
            if (x > 0) {
                csv << ",";
            }
            csv << labels.labels[idx];
        }
        csv << "\n";
    }
}

} // namespace Core::Domain::SpatialPattern
