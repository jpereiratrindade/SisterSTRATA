#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include "core/domain/analysis/PatchAnalysis.hpp"

// Simple helper to create a 3x3 + shape
//   . X .
//   X X X
//   . X .
// Area = 5
// Perimeter = 12 (4 * 3 external edges? No, let's trace it)
// External edges:
// Top cell (1,0): 3 exposed edges
// Left cell (0,1): 3 exposed edges
// Right cell (2,1): 3 exposed edges
// Bottom cell (1,2): 3 exposed edges
// Center cell (1,1): 0 exposed edges
// Total perimeter = 3+3+3+3 = 12. 
// Wait, if diagonal connection is not allowed (4-connectivity).
// Let's assume 4-connectivity is used (PatchAnalysis.cpp uses dirs[4][2]).

void test_cross_shape() {
    using namespace Core::Domain::Analysis;

    // Grid 3x3
    // 0 1 0
    // 1 1 1
    // 0 1 0
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

    auto result = AnalyzeGrid(grid, cfg);

    assert(result.patches.size() == 1);
    const auto& p = result.patches[0];

    std::cout << "Area: " << p.area << " (Expected: 5.0)" << std::endl; 
    assert(std::abs(p.area - 5.0) < 1e-5);

    std::cout << "Perimeter: " << p.perimeter << " (Expected: 12.0)" << std::endl;
    assert(std::abs(p.perimeter - 12.0) < 1e-5);

    // PAR = P / A = 12 / 5 = 2.4
    std::cout << "PAR: " << p.par << " (Expected: 2.4)" << std::endl;
    assert(std::abs(p.par - 2.4) < 1e-5);

    // Shape Index = (0.282 * P) / sqrt(A)
    // SI = (0.282 * 12) / sqrt(5) = 3.384 / 2.2360679 = 1.51336
    double expected_si = (0.282 * 12.0) / std::sqrt(5.0);
    std::cout << "Shape Index: " << p.shape_index << " (Expected: " << expected_si << ")" << std::endl;
    assert(std::abs(p.shape_index - expected_si) < 1e-5);

    std::cout << "Cross Shape Test Passed!" << std::endl;
}

void test_disconnected_shapes() {
    using namespace Core::Domain::Analysis;

    // Grid 3x3
    // 1 0 1
    // 0 0 0
    // 0 0 0
    // Two patches of single pixel
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
    
    auto result = AnalyzeGrid(grid, cfg);
    
    std::cout << "Patches found: " << result.patches.size() << " (Expected: 2)" << std::endl;
    assert(result.patches.size() == 2);
    
    // Each patch: area=1, perimeter=4
    for(const auto& p : result.patches) {
        assert(std::abs(p.area - 1.0) < 1e-5);
        assert(std::abs(p.perimeter - 4.0) < 1e-5);
    }
    
    std::cout << "Disconnected Shapes Test Passed!" << std::endl;
}

int main() {
    test_cross_shape();
    test_disconnected_shapes();
    std::cout << "All PatchAnalysis tests passed." << std::endl;
    return 0;
}
