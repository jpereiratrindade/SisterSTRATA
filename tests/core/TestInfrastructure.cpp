#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "core/domain/energy/EnergyPool.hpp"
#include "core/domain/energy/EnergyAllocationPolicy.hpp"
#include "core/domain/identity/IdentityNode.hpp"
#include "core/domain/seto/SoilMonitorNode.hpp"
#include "core/domain/infrastructure/InfrastructureOrchestrator.hpp"
#include "core/domain/simulation/EnvironmentController.hpp"

namespace {

std::filesystem::path uniqueCsvPath() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("strata_core_infrastructure_test_" + std::to_string(stamp) + ".csv");
}

bool nonNegative(double value) {
    return value >= -1e-9;
}

} // namespace

TEST(CoreInfrastructureTest, RunsDeterministicInfrastructureFlowAndRespectsEnergyInvariants) {
    using namespace strata::domain;

    energy::EnergyPool pool(10000.0, 5000.0);
    energy::EqualitarianPolicy policy;

    identity::IdentityNode identityNode(20.0, 2.0, 5.0);
    seto::SoilMonitorNode soilNode(10.0, 40.0);

    infrastructure::InfrastructureOrchestrator orchestrator(
        pool,
        policy,
        identityNode,
        soilNode
    );

    simulation::EnvironmentController controller(60);
    const std::filesystem::path csvPath = uniqueCsvPath();

    controller.run(orchestrator, csvPath.string());

    ASSERT_TRUE(std::filesystem::exists(csvPath));
    std::ifstream csvIn(csvPath);
    ASSERT_TRUE(csvIn.is_open());

    std::string header;
    std::getline(csvIn, header);
    EXPECT_NE(header.find("IdentityAlloc_Wh"), std::string::npos);
    EXPECT_NE(header.find("SoilAlloc_Wh"), std::string::npos);

    const auto& finalPool = orchestrator.getEnergyPool();
    const auto& finalIdentity = orchestrator.getIdentityNode();
    const auto& finalSoil = orchestrator.getSoilNode();

    EXPECT_TRUE(nonNegative(finalPool.currentStorage()));

    EXPECT_TRUE(nonNegative(finalIdentity.requestedEnergy()));
    EXPECT_TRUE(nonNegative(finalIdentity.allocatedEnergy()));
    EXPECT_TRUE(nonNegative(finalIdentity.consumedEnergy()));
    EXPECT_LE(finalIdentity.allocatedEnergy(), finalIdentity.requestedEnergy() + 1e-9);
    EXPECT_LE(finalIdentity.consumedEnergy(), finalIdentity.allocatedEnergy() + 1e-9);
    EXPECT_GE(finalIdentity.reliabilityIndex(), 0.0);
    EXPECT_LE(finalIdentity.reliabilityIndex(), 1.0);

    EXPECT_TRUE(nonNegative(finalSoil.requestedEnergy()));
    EXPECT_TRUE(nonNegative(finalSoil.allocatedEnergy()));
    EXPECT_TRUE(nonNegative(finalSoil.consumedEnergy()));
    EXPECT_LE(finalSoil.allocatedEnergy(), finalSoil.requestedEnergy() + 1e-9);
    EXPECT_LE(finalSoil.consumedEnergy(), finalSoil.allocatedEnergy() + 1e-9);
    EXPECT_GE(finalSoil.reliabilityIndex(), 0.0);
    EXPECT_LE(finalSoil.reliabilityIndex(), 1.0);

    EXPECT_LE(
        finalIdentity.allocatedEnergy() + finalSoil.allocatedEnergy(),
        finalIdentity.requestedEnergy() + finalSoil.requestedEnergy() + 1e-9
    );

    std::filesystem::remove(csvPath);
}
