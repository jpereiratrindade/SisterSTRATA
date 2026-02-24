#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "core/domain/spatial_pattern/PatchAnalysis.hpp"

TEST(CorePatchAnalysisTest, CrossShapeMetrics) {
    using namespace Core::Domain::SpatialPattern;

    GridData grid;
    grid.width = 3;
    grid.height = 3;
    grid.cellWidth = 1.0;
    grid.cellHeight = 1.0;
    grid.values = {
        0.0, 1.0, 0.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 0.0
    };

    AnalysisConfig cfg;
    cfg.threshold = 0.5;
    cfg.byClass = false;
    cfg.keepLabels = true;

    const auto result = AnalyzeGrid(grid, cfg);

    ASSERT_EQ(result.patches.size(), 1u);
    const auto& p = result.patches[0];

    EXPECT_NEAR(p.area, 5.0, 1e-5);
    EXPECT_NEAR(p.perimeter, 12.0, 1e-5);
    EXPECT_NEAR(p.par, 2.4, 1e-5);

    const double expectedSi = (0.282 * 12.0) / std::sqrt(5.0);
    EXPECT_NEAR(p.shape_index, expectedSi, 1e-5);
}

TEST(CorePatchAnalysisTest, DisconnectedSingleCellPatches) {
    using namespace Core::Domain::SpatialPattern;

    GridData grid;
    grid.width = 3;
    grid.height = 3;
    grid.cellWidth = 1.0;
    grid.cellHeight = 1.0;
    grid.values = {
        1.0, 0.0, 1.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0
    };

    AnalysisConfig cfg;
    cfg.threshold = 0.5;

    const auto result = AnalyzeGrid(grid, cfg);

    ASSERT_EQ(result.patches.size(), 2u);
    for (const auto& p : result.patches) {
        EXPECT_NEAR(p.area, 1.0, 1e-5);
        EXPECT_NEAR(p.perimeter, 4.0, 1e-5);
    }
}
