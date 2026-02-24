#include <cmath>
#include <string>

#include <gtest/gtest.h>

#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectory.hpp"
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectoryService.hpp"

using namespace Core::Domain::FourthDimension::PatchTrajectory;

TEST(CorePatchTrajectoryTest, ComputesBasicTrajectoryMetrics) {
    PatchTrajectory trajectory(1);

    PatchState s0;
    s0.area = 100.0f;
    s0.shapeIndex = 1.0f;
    s0.adjacencyByClass[1] = 100.0f;

    PatchState s1;
    s1.area = 90.0f;
    s1.shapeIndex = 1.1f;
    s1.adjacencyByClass[1] = 90.0f;
    s1.adjacencyByClass[2] = 10.0f;

    trajectory.addState(s0);
    trajectory.addState(s1);

    EXPECT_EQ(trajectory.getLifespan(), 2);
    EXPECT_NEAR(trajectory.getNetAreaTrend(), -10.0f, 0.01f);
    EXPECT_GT(trajectory.getShapeVolatility(), 0.0f);
    EXPECT_LT(trajectory.getStructuralStabilityIndex(), 1.0f);
}

TEST(CorePatchTrajectoryTest, ClassifiesSemanticSummaryForErosiveAndStableCases) {
    PatchTrajectory erosive(1);
    for (int i = 0; i < 3; ++i) {
        PatchState s;
        s.area = 100.0f - (i * 30.0f);
        s.shapeIndex = 1.0f;
        erosive.addState(s);
    }

    std::string summary = PatchTrajectoryService::generateLLMSummary(erosive);
    EXPECT_NE(summary.find("dominant_trajectory_type: Erosiva"), std::string::npos);

    PatchTrajectory stable(2);
    for (int i = 0; i < 3; ++i) {
        PatchState s;
        s.area = 100.0f;
        s.shapeIndex = (i % 2 == 0) ? 1.0f : 1.01f;
        stable.addState(s);
    }

    summary = PatchTrajectoryService::generateLLMSummary(stable);
    EXPECT_NE(summary.find("dominant_trajectory_type: Estável"), std::string::npos);
}

TEST(CorePatchTrajectoryTest, ReportsAdjacencyContrastInSummary) {
    PatchTrajectory trajectory(1);
    PatchState s;
    s.adjacencyByClass[1] = 60.0f;
    s.adjacencyByClass[2] = 40.0f;
    trajectory.addState(s);

    const std::string summary = PatchTrajectoryService::generateLLMSummary(trajectory);
    EXPECT_NE(summary.find("[Classe 1: 60%]"), std::string::npos);
    EXPECT_NE(summary.find("[Classe 2: 40%]"), std::string::npos);
}
