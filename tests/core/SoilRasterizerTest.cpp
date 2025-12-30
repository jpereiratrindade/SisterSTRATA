#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "core/domain/spatial_pattern/SoilRasterizer.hpp"
#include "core/domain/soils/SiBCS.hpp"
#include "world3d/rendering/Vertex.hpp"

using namespace Core::Domain::SpatialPattern;
using namespace Core::Domain::Soils;

void test_simple_grid() {
    std::cout << "Running test_simple_grid..." << std::endl;
    std::vector<World3D::Rendering::Vertex> vertices;
    std::vector<SiBCSClassification> classes;
    
    // V1: (0.5, 0.5) -> Class Latossolo
    World3D::Rendering::Vertex v1; v1.pos = {0.5f, 0.5f, 0.0f};
    SiBCSClassification c1; c1.order = SiBCSOrder::Latossolo;
    vertices.push_back(v1); classes.push_back(c1);

    // V2: (1.5, 0.5) -> Class Argissolo
    World3D::Rendering::Vertex v2; v2.pos = {1.5f, 0.5f, 0.0f};
    SiBCSClassification c2; c2.order = SiBCSOrder::Argissolo;
    vertices.push_back(v2); classes.push_back(c2);

    // V3: (0.5, 1.5) -> Class Latossolo
    World3D::Rendering::Vertex v3; v3.pos = {0.5f, 1.5f, 0.0f};
    SiBCSClassification c3; c3.order = SiBCSOrder::Latossolo;
    vertices.push_back(v3); classes.push_back(c3);

    // V4: (1.5, 1.5) -> Class Argissolo
    World3D::Rendering::Vertex v4; v4.pos = {1.5f, 1.5f, 0.0f};
    SiBCSClassification c4; c4.order = SiBCSOrder::Argissolo;
    vertices.push_back(v4); classes.push_back(c4);

    GridData grid = SoilRasterizer::Rasterize(vertices, classes, 1.0);

    assert(grid.width == 2);
    assert(grid.height == 2);
    assert(std::abs(grid.cellWidth - 1.0) < 0.001);
    
    // Expected IDs
    double idLat = static_cast<double>(static_cast<int>(SiBCSOrder::Latossolo));
    double idArg = static_cast<double>(static_cast<int>(SiBCSOrder::Argissolo));

    // Grid[gy * width + gx]
    // V1 (0.5, 0.5) -> gx=0, gy=0 -> idx 0
    assert(std::abs(grid.values[0] - idLat) < 0.001);
    
    // V2 (1.5, 0.5) -> gx=1, gy=0 -> idx 1
    assert(std::abs(grid.values[1] - idArg) < 0.001);
    
    // V3 (0.5, 1.5) -> gx=0, gy=1 -> idx 2
    assert(std::abs(grid.values[2] - idLat) < 0.001);
    
    // V4 (1.5, 1.5) -> gx=1, gy=1 -> idx 3
    assert(std::abs(grid.values[3] - idArg) < 0.001);

    std::cout << "test_simple_grid Passed!" << std::endl;
}

void test_irregular_spacing() {
    std::cout << "Running test_irregular_spacing..." << std::endl;
    std::vector<World3D::Rendering::Vertex> vertices;
    std::vector<SiBCSClassification> classes;
    
    // V1: (0.1, 0.1) -> Latossolo
    World3D::Rendering::Vertex v1; v1.pos = {0.1f, 0.1f, 0.0f};
    SiBCSClassification c1; c1.order = SiBCSOrder::Latossolo;
    vertices.push_back(v1); classes.push_back(c1);
    
    // V2: (0.2, 0.2) -> Argissolo
    World3D::Rendering::Vertex v2; v2.pos = {0.2f, 0.2f, 0.0f};
    SiBCSClassification c2; c2.order = SiBCSOrder::Argissolo;
    vertices.push_back(v2); classes.push_back(c2);
    
    // Cell size 1.0. Both map to (0,0). Last one (V2) should win.
    GridData grid = SoilRasterizer::Rasterize(vertices, classes, 1.0);
    
    assert(grid.width == 1);
    assert(grid.height == 1);
    
    double idArg = static_cast<double>(static_cast<int>(SiBCSOrder::Argissolo));
    
    assert(std::abs(grid.values[0] - idArg) < 0.001);
    
    std::cout << "test_irregular_spacing Passed!" << std::endl;
}

int main() {
    test_simple_grid();
    test_irregular_spacing();
    std::cout << "All SoilRasterizer tests passed." << std::endl;
    return 0;
}
