#include "core/domain/hydro/HydrologyReport.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include "core/domain/analysis/PatchAnalysis.hpp"

namespace Core::Domain::Hydro {

void HydrologyStats::initRanges() {
    minElevation = 1e9f; maxElevation = -1e9f;
    minSlope = 1e9f; maxSlope = -1e9f;
    minTWI = 1e9f; maxTWI = -1e9f;
    maxFlowAccumulation = -1e9f;
    maxStreamPower = -1e9f;
    saturatedAreaPct = 0.0f;
    avgElevation = 0.0f;
    avgSlope = 0.0f;
    avgTWI = 0.0f;
    streamCount = 0;
    drainageDensity = 0.0f;
    totalDischarge = 0.0f;
}

HydrologyStats HydrologyReport::analyze(const ElevationGrid& terrain,
                                        const HydroGrid& grid,
                                        float resolution,
                                        float streamThreshold) {
    HydrologyStats globalStats;
    globalStats.initRanges();

    if (resolution <= 0.0f) resolution = 1.0f;
    float cellArea = resolution * resolution;

    int w = grid.width;
    int h = grid.height;
    int count = w * h;
    if (w <= 0 || h <= 0 || count <= 0) return globalStats;

    globalStats.id = 0;
    globalStats.areaCells = count;

    struct Accumulator {
        double sumElev = 0.0;
        double sumSlope = 0.0;
        double sumTWI = 0.0;
        int twiCount = 0;
        float streamLength = 0.0f;
        float discharge = 0.0f;
    };

    std::map<int, HydrologyStats> basinStatsMap;
    std::map<int, Accumulator> basinAccMap;

    const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    const float distMult[] = {1.0f, 1.41421356f, 1.0f, 1.41421356f, 1.0f, 1.41421356f, 1.0f, 1.41421356f};

    double g_sumElev = 0.0;
    double g_sumSlope = 0.0;
    double g_sumTWI = 0.0;
    int g_twiCount = 0;
    float g_streamLength = 0.0f;
    float g_discharge = 0.0f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            float elev = terrain.get(x, y);
            float fluxCells = (idx < static_cast<int>(grid.flowAccumulationCells.size()))
                                  ? static_cast<float>(grid.flowAccumulationCells[idx])
                                  : 0.0f;
            int bid = (idx < static_cast<int>(grid.watershedMap.size())) ? grid.watershedMap[idx] : 0;

            // Slope (max drop / distance)
            float maxSlope = 0.0f;
            for (int i = 0; i < 8; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

                float drop = elev - terrain.get(nx, ny);
                if (drop > 0.0f) {
                    float dist = distMult[i] * resolution;
                    float s = drop / dist;
                    if (s > maxSlope) maxSlope = s;
                }
            }

            float specificArea = fluxCells * resolution;
            float tanB = std::max(maxSlope, 0.001f);
            float twi = std::log(std::max(1e-6f, specificArea / tanB));

            bool isStream = (fluxCells >= streamThreshold);
            float localStreamLen = 0.0f;
            if (isStream) {
                int receiver = (idx < static_cast<int>(grid.receiverIndex.size())) ? grid.receiverIndex[idx] : -1;
                if (receiver >= 0 && receiver < count) {
                    int rx = receiver % w;
                    int ry = receiver / w;
                    int dX = std::abs(rx - x);
                    int dY = std::abs(ry - y);
                    float distFactor = (dX + dY == 2) ? 1.41421356f : 1.0f;
                    localStreamLen = distFactor * resolution;
                } else {
                    localStreamLen = resolution;
                }
            }

            // Global stats
            if (elev < globalStats.minElevation) globalStats.minElevation = elev;
            if (elev > globalStats.maxElevation) globalStats.maxElevation = elev;
            g_sumElev += elev;

            if (maxSlope < globalStats.minSlope) globalStats.minSlope = maxSlope;
            if (maxSlope > globalStats.maxSlope) globalStats.maxSlope = maxSlope;
            g_sumSlope += maxSlope;

            float flowArea = fluxCells * cellArea;
            if (flowArea > globalStats.maxFlowAccumulation) globalStats.maxFlowAccumulation = flowArea;

            float spi = specificArea * maxSlope;
            if (spi > globalStats.maxStreamPower) globalStats.maxStreamPower = spi;

            if (twi < globalStats.minTWI) globalStats.minTWI = twi;
            if (twi > globalStats.maxTWI) globalStats.maxTWI = twi;
            g_sumTWI += twi;
            g_twiCount++;
            if (twi > 8.0f) globalStats.saturatedAreaPct += 1.0f;

            g_streamLength += localStreamLen;
            if (isStream) g_discharge += flowArea;

            // Basin stats
            if (bid > 0) {
                if (basinStatsMap.find(bid) == basinStatsMap.end()) {
                    basinStatsMap[bid].initRanges();
                    basinStatsMap[bid].id = bid;
                    basinStatsMap[bid].areaCells = 0;
                }

                HydrologyStats& bStats = basinStatsMap[bid];
                Accumulator& bAcc = basinAccMap[bid];

                bStats.areaCells++;

                if (elev < bStats.minElevation) bStats.minElevation = elev;
                if (elev > bStats.maxElevation) bStats.maxElevation = elev;
                bAcc.sumElev += elev;

                if (maxSlope < bStats.minSlope) bStats.minSlope = maxSlope;
                if (maxSlope > bStats.maxSlope) bStats.maxSlope = maxSlope;
                bAcc.sumSlope += maxSlope;

                if (flowArea > bStats.maxFlowAccumulation) bStats.maxFlowAccumulation = flowArea;
                if (spi > bStats.maxStreamPower) bStats.maxStreamPower = spi;

                if (twi < bStats.minTWI) bStats.minTWI = twi;
                if (twi > bStats.maxTWI) bStats.maxTWI = twi;
                bAcc.sumTWI += twi;
                bAcc.twiCount++;
                if (twi > 8.0f) bStats.saturatedAreaPct += 1.0f;

                bAcc.streamLength += localStreamLen;
                if (isStream) bAcc.discharge += flowArea;
            }
        }
    }

    globalStats.avgElevation = static_cast<float>(g_sumElev / count);
    globalStats.avgSlope = static_cast<float>(g_sumSlope / count);
    if (g_twiCount > 0) globalStats.avgTWI = static_cast<float>(g_sumTWI / g_twiCount);
    globalStats.saturatedAreaPct = (globalStats.saturatedAreaPct / static_cast<float>(count)) * 100.0f;

    float totalAreaM2 = count * cellArea;
    if (totalAreaM2 > 0.0f) {
        globalStats.drainageDensity = g_streamLength / totalAreaM2;
    }
    globalStats.streamCount = static_cast<int>(g_streamLength / resolution);
    globalStats.totalDischarge = g_discharge;

    std::vector<HydrologyStats> allBasins;
    for (auto& kv : basinStatsMap) {
        int bid = kv.first;
        HydrologyStats& bs = kv.second;
        const Accumulator& acc = basinAccMap[bid];

        if (bs.areaCells > 0) {
            bs.avgElevation = static_cast<float>(acc.sumElev / bs.areaCells);
            bs.avgSlope = static_cast<float>(acc.sumSlope / bs.areaCells);
            if (acc.twiCount > 0) bs.avgTWI = static_cast<float>(acc.sumTWI / acc.twiCount);

            bs.saturatedAreaPct = (bs.saturatedAreaPct / static_cast<float>(bs.areaCells)) * 100.0f;
            float basinAreaM2 = bs.areaCells * cellArea;
            if (basinAreaM2 > 0.0f) {
                bs.drainageDensity = acc.streamLength / basinAreaM2;
            }
            bs.streamCount = static_cast<int>(acc.streamLength / resolution);
            bs.totalDischarge = acc.discharge;
            allBasins.push_back(bs);
        }
    }

    if (!allBasins.empty()) {
        globalStats.basinCount = static_cast<int>(allBasins.size());
        std::sort(allBasins.begin(), allBasins.end(), [](const HydrologyStats& a, const HydrologyStats& b) {
            return a.areaCells > b.areaCells;
        });

        globalStats.largestBasinArea = allBasins[0].areaCells;
        globalStats.largestBasinPct = (float(allBasins[0].areaCells) / float(count)) * 100.0f;

        int keep = std::min(static_cast<int>(allBasins.size()), 3);
        for (int i = 0; i < keep; ++i) {
            globalStats.topBasins.push_back(allBasins[i]);
        }
        globalStats.allBasins = allBasins;
    }

    return globalStats;
}

bool HydrologyReport::generateToFile(const ElevationGrid& terrain,
                                     const HydroGrid& grid,
                                     float resolution,
                                     const std::string& filepath,
                                     float streamThreshold) {
    HydrologyStats stats = analyze(terrain, grid, resolution, streamThreshold);

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "Slope (m/m):\n";
    out << "  - Method:              Steepest Descent (Max Drop / Distance)\n";
    out << "  - Mean:                " << stats.avgSlope << " (" << (stats.avgSlope * 100.0f) << "%)\n";
    out << "  - Max:                 " << stats.maxSlope << " (" << (stats.maxSlope * 100.0f) << "%)\n\n";

    out << "Hydrologic Parameters\n";
    out << "-----------------------------------------------------------------\n";
    out << "Flow Accumulation Area:\n";
    out << "  - Max:                 " << stats.maxFlowAccumulation << " m2\n\n";

    out << "Stream Power (SPI ~ A_spec * S):\n";
    out << "  - Max:                 " << stats.maxStreamPower << "\n\n";

    out << "Topographic Wetness Index (TWI = ln(a / tanB)):\n";
    out << "  - Min:                 " << stats.minTWI << "\n";
    out << "  - Max:                 " << stats.maxTWI << "\n";
    out << "  - Mean:                " << stats.avgTWI << "\n";
    out << "  - Area with TWI > 8:   " << stats.saturatedAreaPct << " %\n\n";

    out << "Drainage Network\n";
    out << "-----------------------------------------------------------------\n";
    out << "Channel Threshold: Flow (Cells) > " << streamThreshold << "\n";
    out << "Drainage Density:\n";
    out << "  - Density:             " << std::scientific << stats.drainageDensity << " m/m2 (m-1)\n";
    if (stats.drainageDensity > 0.0f) {
        out << "  - Equivalent:          " << std::fixed << std::setprecision(2) << (stats.drainageDensity * 1000.0f) << " Km of rivers per Km2\n";
    }
    out << "  - Total Length:        " << (stats.streamCount * resolution) << " m (approx)\n\n";

    if (stats.basinCount > 0) {
        out << "Basins (Watershed Segmentation)\n";
        out << "-----------------------------------------------------------------\n";
        out << std::fixed << std::setprecision(0);
        out << "Total Basins:            " << stats.basinCount << "\n";
        out << "Largest Basin Area:      " << (stats.largestBasinArea * resolution * resolution) << " m2\n";
        out << std::fixed << std::setprecision(2);
        out << "Largest Basin Dominance: " << stats.largestBasinPct << " % of total area\n\n";

        out << "All Basins\n";
        out << "-----------------------------------------------------------------\n";

        int bIndex = 1;
        std::vector<HydrologyStats> basins = stats.allBasins;
        std::sort(basins.begin(), basins.end(), [](const HydrologyStats& a, const HydrologyStats& b) {
            return a.id < b.id;
        });

        for (const auto& basin : basins) {
            float bAreaM2 = basin.areaCells * resolution * resolution;
            out << bIndex << ". Basin ID " << basin.id << " (Area: " << bAreaM2 << " m2)\n";
            out << "   - Elevation (Min/Mean/Max): " << basin.minElevation << " / " << basin.avgElevation << " / " << basin.maxElevation << "\n";
            out << "   - Mean Slope:               " << basin.avgSlope << " (" << (basin.avgSlope * 100.0f) << "%)\n";
            out << "   - Mean TWI:                 " << basin.avgTWI << "\n";
            out << "   - Saturation (TWI>8):       " << basin.saturatedAreaPct << " %\n";
            out << "   - Drainage Density:         " << std::scientific << basin.drainageDensity << std::fixed << " m-1\n";
            out << "   - Max Stream Power:         " << basin.maxStreamPower << "\n\n";
            bIndex++;
        }
    }

    if (grid.watershedMap.size() == static_cast<size_t>(grid.width * grid.height)) {
        Core::Domain::Analysis::GridData gridData;
        gridData.width = grid.width;
        gridData.height = grid.height;
        gridData.cellWidth = resolution;
        gridData.cellHeight = resolution;
        gridData.values.resize(static_cast<size_t>(grid.width * grid.height));
        for (int i = 0; i < grid.width * grid.height; ++i) {
            gridData.values[static_cast<size_t>(i)] = static_cast<double>(grid.watershedMap[i]);
        }

        Core::Domain::Analysis::AnalysisConfig cfg;
        cfg.byClass = true;
        Core::Domain::Analysis::AnalysisResult patchResult = Core::Domain::Analysis::AnalyzeGrid(gridData, cfg);

        out << "Patch Analysis (Basins)\n";
        out << "-----------------------------------------------------------------\n";
        out << "Patches:                " << patchResult.summary.patchCount << "\n";
        out << "Mean PAR:               " << patchResult.summary.meanPar << "\n";
        out << "Mean Shape Index:       " << patchResult.summary.meanShapeIndex << "\n";
        out << "Mean RCC:               " << patchResult.summary.meanRcc << "\n";
        out << "S1:                     " << patchResult.summary.s1 << "\n";
        out << "S2:                     " << patchResult.summary.s2 << "\n\n";
    }

    out << "=================================================================\n";
    out << "End of report.\n";

    out.close();
    return true;
}

} // namespace Core::Domain::Hydro
