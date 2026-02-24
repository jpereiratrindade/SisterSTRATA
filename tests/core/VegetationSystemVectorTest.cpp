#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/vegetation/EcologicalScenario.hpp"
#include "core/domain/vegetation/VegetationMappingService.hpp"
#include "core/domain/vegetation/VegetationType.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/value_objects/TerrainVertex.hpp"

using namespace Core::Domain::Vegetation;
using namespace Core::Domain::FourthDimension;

TEST(CoreVegVectorTest, GroupsHypothesesByScenarioId) {
    VegetationSystemOriginal system;

    ReliefCondition cond1;
    cond1.minSlope = 0.0f;
    cond1.maxSlope = 10.0f;
    VegetationOriginal h1(HypothesisID("Hypothesis_A"), VegetationType(VegetationCode::Agua), cond1);

    ReliefCondition cond2;
    cond2.minSlope = 10.0f;
    cond2.maxSlope = 30.0f;
    VegetationOriginal h2(HypothesisID("Hypothesis_A"), VegetationType(VegetationCode::FlorestalNatural), cond2);

    ReliefCondition cond3;
    cond3.minSlope = 0.0f;
    cond3.maxSlope = 45.0f;
    VegetationOriginal h3(HypothesisID("Hypothesis_B"), VegetationType(VegetationCode::Campestre), cond3);

    system.addHypothesis(h1);
    system.addHypothesis(h2);
    system.addHypothesis(h3);

    const auto& scenarios = system.getScenarios();
    ASSERT_EQ(scenarios.size(), 2u);

    bool foundA = false;
    for (const auto& s : scenarios) {
        if (s.getId() == "Hypothesis_A") {
            EXPECT_EQ(s.getComponents().size(), 2u);
            foundA = true;
        }
    }
    EXPECT_TRUE(foundA);
}

TEST(CoreVegVectorTest, ResolvesScenarioToExpectedVegetationCodes) {
    EcologicalScenario scenario("Test_Vector");

    ReliefCondition cond1;
    cond1.minSlope = 0.0f;
    cond1.maxSlope = 5.0f;
    scenario.addComponent(VegetationOriginal(HypothesisID("Test_Vector"), VegetationType(VegetationCode::Agua), cond1));

    ReliefCondition cond2;
    cond2.minSlope = 5.0f;
    cond2.maxSlope = 30.0f;
    scenario.addComponent(VegetationOriginal(HypothesisID("Test_Vector"), VegetationType(VegetationCode::FlorestalNatural), cond2));

    std::vector<Core::ValueObjects::TerrainVertex> vertices;
    Core::ValueObjects::TerrainVertex v0;
    v0.normal = glm::vec3(0, 0, 1);
    vertices.push_back(v0);

    Core::ValueObjects::TerrainVertex v1;
    v1.normal = glm::vec3(0, 0.25f, 0.968f);
    vertices.push_back(v1);

    Core::Domain::Hydro::HydroGrid hydro;
    const auto codes = VegetationMappingService::resolveScenarioToCodes(scenario, vertices, hydro, 1.0f);

    ASSERT_EQ(codes.size(), 2u);
    EXPECT_EQ(codes[0], static_cast<int>(VegetationCode::Agua));
    EXPECT_EQ(codes[1], static_cast<int>(VegetationCode::FlorestalNatural));
}
