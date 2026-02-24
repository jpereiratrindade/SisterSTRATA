#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "core/domain/hydro/DrainageSystem.hpp"
#include "core/domain/hydro/HydrologyReport.hpp"
#include "core/domain/hydro/Watershed.hpp"

namespace {

bool nearlyEqual(float lhs, float rhs, float eps = 1e-5f) {
    return std::fabs(lhs - rhs) <= eps;
}

Core::Domain::Hydro::ElevationGrid buildBowlTerrain3x3() {
    Core::Domain::Hydro::ElevationGrid terrain{};
    terrain.width = 3;
    terrain.height = 3;
    terrain.z = {
        3.0f, 3.0f, 3.0f,
        3.0f, 1.0f, 3.0f,
        3.0f, 3.0f, 3.0f,
    };
    return terrain;
}

void testDrainageAndWatershed() {
    using namespace Core::Domain::Hydro;

    const ElevationGrid terrain = buildBowlTerrain3x3();
    HydroGrid grid{};
    DrainageSystem::process(terrain, grid, SinkHandling::Ignore);

    assert(grid.width == 3);
    assert(grid.height == 3);
    assert(grid.flowDirection.size() == 9);
    assert(grid.receiverIndex.size() == 9);
    assert(grid.flowAccumulationCells.size() == 9);

    const int center = 4;
    assert(grid.receiverIndex[center] == -1);
    assert(grid.flowDirection[center] == FlowDir::Sink);
    assert(grid.flowAccumulationCells[center] == 9);

    const int basins = Watershed::segmentGlobal(grid);
    assert(basins == 1);
    for (int basinId : grid.watershedMap) {
        assert(basinId == 1);
    }

    const auto delineated = Watershed::delineate(grid, 1, 1, 1);
    assert(delineated.size() == 9);
    for (uint8_t cell : delineated) {
        assert(cell == 255);
    }

    const auto boundaryMask = Watershed::computeBoundaryMask(grid);
    assert(boundaryMask.size() == 9);
    for (uint8_t edge : boundaryMask) {
        assert(edge == 0);
    }
}

void testHydrologyReportAnalysisAndFile() {
    using namespace Core::Domain::Hydro;

    const ElevationGrid terrain = buildBowlTerrain3x3();
    HydroGrid grid{};
    DrainageSystem::process(terrain, grid, SinkHandling::Ignore);
    Watershed::segmentGlobal(grid);

    const HydrologyStats stats = HydrologyReport::analyze(terrain, grid, 1.0f, 1.0f);
    assert(stats.areaCells == 9);
    assert(stats.basinCount == 1);
    assert(stats.largestBasinArea == 9);
    assert(nearlyEqual(stats.minElevation, 1.0f));
    assert(nearlyEqual(stats.maxElevation, 3.0f));
    assert(nearlyEqual(stats.maxFlowAccumulation, 9.0f));

    const float expectedAvgElevation = 25.0f / 9.0f;
    assert(nearlyEqual(stats.avgElevation, expectedAvgElevation));

    const std::filesystem::path outPath =
        std::filesystem::temp_directory_path() / "hydrology_report_test.txt";
    const bool generated = HydrologyReport::generateToFile(
        terrain, grid, 1.0f, outPath.string(), 1.0f);
    assert(generated);
    assert(std::filesystem::exists(outPath));
    std::filesystem::remove(outPath);
}

} // namespace

int main() {
    testDrainageAndWatershed();
    testHydrologyReportAnalysisAndFile();
    std::cout << "Hydro domain tests passed.\n";
    return 0;
}
