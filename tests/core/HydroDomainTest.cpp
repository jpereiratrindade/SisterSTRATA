#include <cmath>
#include <filesystem>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "core/domain/hydro/DrainageSystem.hpp"
#include "core/domain/hydro/HydrologyReport.hpp"
#include "core/domain/hydro/Watershed.hpp"

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

TEST(HydroDomainTest, DrainageAndWatershedFlow) {
    using namespace Core::Domain::Hydro;

    const ElevationGrid terrain = buildBowlTerrain3x3();
    HydroGrid grid{};
    DrainageSystem::process(terrain, grid, SinkHandling::Ignore);

    EXPECT_EQ(grid.width, 3);
    EXPECT_EQ(grid.height, 3);
    ASSERT_EQ(grid.flowDirection.size(), 9u);
    ASSERT_EQ(grid.receiverIndex.size(), 9u);
    ASSERT_EQ(grid.flowAccumulationCells.size(), 9u);

    const int center = 4;
    EXPECT_EQ(grid.receiverIndex[center], -1);
    EXPECT_EQ(grid.flowDirection[center], FlowDir::Sink);
    EXPECT_EQ(grid.flowAccumulationCells[center], 9);

    const int basins = Watershed::segmentGlobal(grid);
    EXPECT_EQ(basins, 1);
    for (int basinId : grid.watershedMap) {
        EXPECT_EQ(basinId, 1);
    }

    const auto delineated = Watershed::delineate(grid, 1, 1, 1);
    ASSERT_EQ(delineated.size(), 9u);
    for (uint8_t cell : delineated) {
        EXPECT_EQ(cell, 255);
    }

    const auto boundaryMask = Watershed::computeBoundaryMask(grid);
    ASSERT_EQ(boundaryMask.size(), 9u);
    for (uint8_t edge : boundaryMask) {
        EXPECT_EQ(edge, 0);
    }
}

TEST(HydroDomainTest, HydrologyReportAnalysisAndFile) {
    using namespace Core::Domain::Hydro;

    const ElevationGrid terrain = buildBowlTerrain3x3();
    HydroGrid grid{};
    DrainageSystem::process(terrain, grid, SinkHandling::Ignore);
    Watershed::segmentGlobal(grid);

    const HydrologyStats stats = HydrologyReport::analyze(terrain, grid, 1.0f, 1.0f);
    EXPECT_EQ(stats.areaCells, 9);
    EXPECT_EQ(stats.basinCount, 1);
    EXPECT_EQ(stats.largestBasinArea, 9);
    EXPECT_TRUE(nearlyEqual(stats.minElevation, 1.0f));
    EXPECT_TRUE(nearlyEqual(stats.maxElevation, 3.0f));
    EXPECT_TRUE(nearlyEqual(stats.maxFlowAccumulation, 9.0f));

    const float expectedAvgElevation = 25.0f / 9.0f;
    EXPECT_TRUE(nearlyEqual(stats.avgElevation, expectedAvgElevation));

    const std::filesystem::path outPath =
        std::filesystem::temp_directory_path() / "hydrology_report_test.txt";
    const bool generated = HydrologyReport::generateToFile(
        terrain, grid, 1.0f, outPath.string(), 1.0f);
    EXPECT_TRUE(generated);
    EXPECT_TRUE(std::filesystem::exists(outPath));
    std::filesystem::remove(outPath);
}
