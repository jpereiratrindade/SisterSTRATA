#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <string>
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectory.hpp"
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectoryService.hpp"

using namespace Core::Domain::FourthDimension::PatchTrajectory;

void testBasicMetrics() {
    std::cout << "Testing Basic Metrics...\n";
    PatchTrajectory trajectory(1);
    
    // State 0: Large, stable
    PatchState s0;
    s0.area = 100.0f;
    s0.shapeIndex = 1.0f;
    s0.adjacencyByClass[1] = 100.0f;
    
    // State 1: Slightly smaller
    PatchState s1;
    s1.area = 90.0f;
    s1.shapeIndex = 1.1f;
    s1.adjacencyByClass[1] = 90.0f;
    s1.adjacencyByClass[2] = 10.0f;
    
    trajectory.addState(s0);
    trajectory.addState(s1);
    
    assert(trajectory.getLifespan() == 2);
    assert(std::abs(trajectory.getNetAreaTrend() - (-10.0f)) < 0.01f);
    assert(trajectory.getShapeVolatility() > 0.0f);
    assert(trajectory.getStructuralStabilityIndex() < 1.0f);
    std::cout << "Basic Metrics: OK\n";
}

void testSemanticClassification() {
    std::cout << "Testing Semantic Classification...\n";
    // Erosive Case
    PatchTrajectory erosive(1);
    for (int i=0; i<3; ++i) {
        PatchState s;
        s.area = 100.0f - (i * 30.0f); // 100, 70, 40 -> Trend -60
        s.shapeIndex = 1.0f;
        erosive.addState(s);
    }
    
    std::string summary = PatchTrajectoryService::generateLLMSummary(erosive);
    assert(summary.find("dominant_trajectory_type: Erosiva") != std::string::npos);
    
    // Stable Case
    PatchTrajectory stable(2);
    for (int i=0; i<3; ++i) {
        PatchState s;
        s.area = 100.0f;
        s.shapeIndex = i % 2 == 0 ? 1.0f : 1.01f; // Very low volatility
        stable.addState(s);
    }
    summary = PatchTrajectoryService::generateLLMSummary(stable);
    assert(summary.find("dominant_trajectory_type: Estável") != std::string::npos);
    std::cout << "Semantic Classification: OK\n";
}

void testAdjacencyContrast() {
    std::cout << "Testing Adjacency Contrast...\n";
    PatchTrajectory trajectory(1);
    PatchState s;
    s.adjacencyByClass[1] = 60.0f; // Forest
    s.adjacencyByClass[2] = 40.0f; // Agriculture
    trajectory.addState(s);
    
    std::string summary = PatchTrajectoryService::generateLLMSummary(trajectory);
    assert(summary.find("[Classe 1: 60%]") != std::string::npos);
    assert(summary.find("[Classe 2: 40%]") != std::string::npos);
    std::cout << "Adjacency Contrast: OK\n";
}

int main() {
    try {
        testBasicMetrics();
        testSemanticClassification();
        testAdjacencyContrast();
        std::cout << "All PatchTrajectory tests passed!\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
