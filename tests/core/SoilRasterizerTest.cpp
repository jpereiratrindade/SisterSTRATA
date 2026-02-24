#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "core/domain/spatial_pattern/SoilRasterizer.hpp"
#include "core/domain/soils/SiBCS.hpp"
#include "core/value_objects/TerrainVertex.hpp"

using namespace Core::Domain::SpatialPattern;
using namespace Core::Domain::Soils;

TEST(CoreSoilRasterizerTest, RasterizesSimpleGrid) {
    std::vector<Core::ValueObjects::TerrainVertex> vertices;
    std::vector<SiBCSClassification> classes;

    Core::ValueObjects::TerrainVertex v1; v1.pos = {0.5f, 0.5f, 0.0f};
    SiBCSClassification c1; c1.order = SiBCSOrder::Latossolo;
    vertices.push_back(v1); classes.push_back(c1);

    Core::ValueObjects::TerrainVertex v2; v2.pos = {1.5f, 0.5f, 0.0f};
    SiBCSClassification c2; c2.order = SiBCSOrder::Argissolo;
    vertices.push_back(v2); classes.push_back(c2);

    Core::ValueObjects::TerrainVertex v3; v3.pos = {0.5f, 1.5f, 0.0f};
    SiBCSClassification c3; c3.order = SiBCSOrder::Latossolo;
    vertices.push_back(v3); classes.push_back(c3);

    Core::ValueObjects::TerrainVertex v4; v4.pos = {1.5f, 1.5f, 0.0f};
    SiBCSClassification c4; c4.order = SiBCSOrder::Argissolo;
    vertices.push_back(v4); classes.push_back(c4);

    const GridData grid = SoilRasterizer::Rasterize(vertices, classes, 1.0);

    ASSERT_EQ(grid.width, 2);
    ASSERT_EQ(grid.height, 2);
    EXPECT_NEAR(grid.cellWidth, 1.0, 0.001);

    const double idLat = static_cast<double>(static_cast<int>(SiBCSOrder::Latossolo));
    const double idArg = static_cast<double>(static_cast<int>(SiBCSOrder::Argissolo));

    EXPECT_NEAR(grid.values[0], idLat, 0.001);
    EXPECT_NEAR(grid.values[1], idArg, 0.001);
    EXPECT_NEAR(grid.values[2], idLat, 0.001);
    EXPECT_NEAR(grid.values[3], idArg, 0.001);
}

TEST(CoreSoilRasterizerTest, LastVertexWinsSameCellOnIrregularSpacing) {
    std::vector<Core::ValueObjects::TerrainVertex> vertices;
    std::vector<SiBCSClassification> classes;

    Core::ValueObjects::TerrainVertex v1; v1.pos = {0.1f, 0.1f, 0.0f};
    SiBCSClassification c1; c1.order = SiBCSOrder::Latossolo;
    vertices.push_back(v1); classes.push_back(c1);

    Core::ValueObjects::TerrainVertex v2; v2.pos = {0.2f, 0.2f, 0.0f};
    SiBCSClassification c2; c2.order = SiBCSOrder::Argissolo;
    vertices.push_back(v2); classes.push_back(c2);

    const GridData grid = SoilRasterizer::Rasterize(vertices, classes, 1.0);

    ASSERT_EQ(grid.width, 1);
    ASSERT_EQ(grid.height, 1);

    const double idArg = static_cast<double>(static_cast<int>(SiBCSOrder::Argissolo));
    EXPECT_NEAR(grid.values[0], idArg, 0.001);
}
