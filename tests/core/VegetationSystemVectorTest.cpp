#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/vegetation/EcologicalScenario.hpp"
#include "core/domain/vegetation/VegetationMappingService.hpp"
#include "core/domain/vegetation/VegetationType.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "world3d/rendering/Vertex.hpp"

using namespace Core::Domain::Vegetation;
using namespace Core::Domain::FourthDimension;

void testScenarioGrouping() {
    std::cout << "Testing Scenario Grouping...\n";
    VegetationSystemOriginal system;

    // Component 1 for Hypothesis_A
    ReliefCondition cond1;
    cond1.minSlope = 0.0f;
    cond1.maxSlope = 10.0f;
    VegetationOriginal h1(HypothesisID("Hypothesis_A"), VegetationType(VegetationCode::Agua), cond1);

    // Component 2 for Hypothesis_A
    ReliefCondition cond2;
    cond2.minSlope = 10.0f;
    cond2.maxSlope = 30.0f;
    VegetationOriginal h2(HypothesisID("Hypothesis_A"), VegetationType(VegetationCode::FlorestalNatural), cond2);

    // Component 1 for Hypothesis_B
    ReliefCondition cond3;
    cond3.minSlope = 0.0f;
    cond3.maxSlope = 45.0f;
    VegetationOriginal h3(HypothesisID("Hypothesis_B"), VegetationType(VegetationCode::Campestre), cond3);

    system.addHypothesis(h1);
    system.addHypothesis(h2);
    system.addHypothesis(h3);

    const auto& scenarios = system.getScenarios();
    assert(scenarios.size() == 2);
    
    // Check Hypothesis_A has 2 components
    bool foundA = false;
    for (const auto& s : scenarios) {
        if (s.getId() == "Hypothesis_A") {
            assert(s.getComponents().size() == 2);
            foundA = true;
        }
    }
    assert(foundA);

    std::cout << "Scenario Grouping: OK\n";
}

void testScenarioResolution() {
    std::cout << "Testing Scenario Resolution...\n";
    
    EcologicalScenario scenario("Test_Vector");
    
    // Component 1: Water on flat ground
    ReliefCondition cond1;
    cond1.minSlope = 0.0f;
    cond1.maxSlope = 5.0f;
    scenario.addComponent(VegetationOriginal(HypothesisID("Test_Vector"), VegetationType(VegetationCode::Agua), cond1));

    // Component 2: Forest on steeper ground
    ReliefCondition cond2;
    cond2.minSlope = 5.0f;
    cond2.maxSlope = 30.0f;
    scenario.addComponent(VegetationOriginal(HypothesisID("Test_Vector"), VegetationType(VegetationCode::FlorestalNatural), cond2));

    // Setup mock vertices
    std::vector<World3D::Rendering::Vertex> vertices;
    // Vertex 0: Flat (Up normal)
    World3D::Rendering::Vertex v0;
    v0.normal = glm::vec3(0,0,1); // 0 degrees
    vertices.push_back(v0);
    
    // Vertex 1: Sloped
    World3D::Rendering::Vertex v1;
    v1.normal = glm::vec3(0, 0.25f, 0.968f); // ~14.5 degrees
    vertices.push_back(v1);

    Core::Domain::Hydro::HydroGrid hydro; // Empty
    float spacing = 1.0f;

    auto codes = VegetationMappingService::resolveScenarioToCodes(scenario, vertices, hydro, spacing);
    
    assert(codes.size() == 2);
    assert(codes[0] == static_cast<int>(VegetationCode::Agua));
    assert(codes[1] == static_cast<int>(VegetationCode::FlorestalNatural));

    std::cout << "Scenario Resolution: OK\n";
}

int main() {
    try {
        testScenarioGrouping();
        testScenarioResolution();
        std::cout << "All Vegetation Vector tests passed!\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
