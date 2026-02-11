#include <gtest/gtest.h>

#include "application/services/SimulationService.hpp"
#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"

using Application::Services::SimulationService;

class SimulationServiceTest : public ::testing::Test {
protected:
    Core::Domain::Workspace workspace;
    Core::Domain::FourthDimension::Trajectory trajectory;

    void SetUp() override {
        // Create a small world for simulation
        workspace.createWorld("Test Environment", 10, 10);
    }
};

TEST_F(SimulationServiceTest, StabilityProducesTwoIdenticalSlices) {
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Stability, workspace, trajectory);

    const auto& slices = trajectory.getTimeSlices();
    ASSERT_EQ(slices.size(), 2u);

    // Both slices should have identical cover (all forest = 1)
    const auto& cover1 = slices[0].getEcologicalCoverState();
    const auto& cover2 = slices[1].getEcologicalCoverState();
    ASSERT_EQ(cover1.size(), cover2.size());

    for (size_t i = 0; i < cover1.size(); ++i) {
        EXPECT_EQ(cover1[i], cover2[i]) << "Mismatch at index " << i;
    }
}

TEST_F(SimulationServiceTest, FragmentationProducesCheckerboard) {
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Fragmentation, workspace, trajectory);

    const auto& slices = trajectory.getTimeSlices();
    ASSERT_EQ(slices.size(), 2u);

    // First slice should be all forest
    const auto& baseline = slices[0].getEcologicalCoverState();
    for (int val : baseline) {
        EXPECT_EQ(val, 1);
    }

    // Second slice should have mixed values (forest=1 and soil=-1)
    const auto& fragmented = slices[1].getEcologicalCoverState();
    bool hasForest = false, hasSoil = false;
    for (int val : fragmented) {
        if (val == 1) hasForest = true;
        if (val == -1) hasSoil = true;
    }
    EXPECT_TRUE(hasForest);
    EXPECT_TRUE(hasSoil);
}

TEST_F(SimulationServiceTest, DeforestationRemovesMostCover) {
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Deforestation, workspace, trajectory);

    const auto& slices = trajectory.getTimeSlices();
    ASSERT_EQ(slices.size(), 2u);

    // Count remaining forest in deforested slice
    const auto& deforested = slices[1].getEcologicalCoverState();
    size_t forestCount = 0;
    for (int val : deforested) {
        if (val == 1) ++forestCount;
    }

    // Should have ~10% forest remaining
    double forestRatio = static_cast<double>(forestCount) / deforested.size();
    EXPECT_LT(forestRatio, 0.15);
    EXPECT_GT(forestRatio, 0.05);
}

TEST_F(SimulationServiceTest, ClearsTrajectoryBeforeSimulation) {
    // Run stability first
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Stability, workspace, trajectory);
    EXPECT_EQ(trajectory.getTimeSlices().size(), 2u);

    // Run fragmentation — should clear previous slices
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Fragmentation, workspace, trajectory);
    EXPECT_EQ(trajectory.getTimeSlices().size(), 2u); // Not 4
}

TEST_F(SimulationServiceTest, AutoCreatesWorldIfMissing) {
    Core::Domain::Workspace emptyWorkspace;
    Core::Domain::FourthDimension::Trajectory emptyTrajectory;

    // Should not crash even without a world
    SimulationService::simulateCondition(
        SimulationService::SimulationType::Stability, emptyWorkspace, emptyTrajectory);

    // World should have been auto-created
    EXPECT_NE(emptyWorkspace.getWorld().get(), nullptr);
    EXPECT_EQ(emptyTrajectory.getTimeSlices().size(), 2u);
}
