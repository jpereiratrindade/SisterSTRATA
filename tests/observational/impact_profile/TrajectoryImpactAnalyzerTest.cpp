#include <gtest/gtest.h>
#include "src/observational/impact_profile/infrastructure/TrajectoryImpactAnalyzerImpl.hpp"
#include "src/core/domain/fourth_dimension/Trajectory.hpp"

using namespace SisterSTRATA::Observational::ImpactProfile::Infrastructure;
using namespace SisterSTRATA::Observational::ImpactProfile::Domain;
using namespace Core::Domain::FourthDimension;

class TrajectoryImpactAnalyzerTest : public ::testing::Test {
protected:
    // Helper to create a trajectory with a single slice of specific pattern
    Trajectory createTrajectory(const std::vector<int>& cover, int id_offset) {
        Trajectory t;
        // Mock water mask (all false) and metadata
        std::vector<bool> water(cover.size(), false);
        TimeSlice slice(id_offset, 1, cover, water, "MockSlice");
        t.addTimeSlice(slice);
        return t;
    }
};

TEST_F(TrajectoryImpactAnalyzerTest, AnalyzeIdenticalTrajectoriesReturnsZeroDeviations) {
    auto analyzer = TrajectoryImpactAnalyzerImpl(10, 10); // 10x10 Grid
    
    // Create identical 10x10 cover (all 1s)
    std::vector<int> cover(100, 1);
    
    auto t1 = createTrajectory(cover, 1);
    auto t2 = createTrajectory(cover, 2);

    ReferenceFrame context(ReferenceType::ControlGroup, "Self Check", "T1");

    auto profile = analyzer.analyze(t1, t2, context);

    EXPECT_NEAR(profile.getStructuralDeviation().fragmentationIndexDelta, 0.0, 0.001);
    EXPECT_NEAR(profile.getStructuralDeviation().spatialCoherenceDelta, 0.0, 0.001);
    EXPECT_EQ(profile.getTemporalPattern().trend, DeviationTrend::Parallel);
}

TEST_F(TrajectoryImpactAnalyzerTest, AnalyzeFragmentationShift) {
    auto analyzer = TrajectoryImpactAnalyzerImpl(4, 4); // 4x4 Grid
    
    // Reference: Solid block (Coherent)
    // 1 1 1 1
    // 1 1 1 1
    // 0 0 0 0
    // 0 0 0 0
    std::vector<int> refCover = {
        1,1,1,1,
        1,1,1,1,
        0,0,0,0,
        0,0,0,0
    };

    // Observed: Checkerboard (Fragmented)
    // 1 0 1 0
    // 0 1 0 1
    // 0 0 0 0
    // 0 0 0 0
    std::vector<int> obsCover = {
        1,0,1,0,
        0,1,0,1,
        0,0,0,0,
        0,0,0,0
    };

    auto ref = createTrajectory(refCover, 1);
    auto obs = createTrajectory(obsCover, 2);
    ReferenceFrame context(ReferenceType::Historical, "History", "REF");

    auto profile = analyzer.analyze(obs, ref, context);

    // Observed (Mean SI = 1.0 for squares) < Reference (Mean SI > 1.0 for rectangle)
    // So Delta = Obs - Ref should be NEGATIVE
    EXPECT_LT(profile.getStructuralDeviation().fragmentationIndexDelta, 0.0);
}
