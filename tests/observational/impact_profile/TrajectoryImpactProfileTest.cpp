#include <gtest/gtest.h>
#include "src/observational/impact_profile/domain/entities/TrajectoryImpactProfile.hpp"
#include "src/observational/impact_profile/domain/value_objects/ReferenceFrame.hpp"
#include "src/observational/impact_profile/domain/value_objects/StructuralDeviation.hpp"
#include "src/observational/impact_profile/domain/value_objects/TemporalDeviationPattern.hpp"

using namespace SisterSTRATA::Observational::ImpactProfile::Domain;

TEST(TrajectoryImpactProfileTest, ImmutableCreation) {
    // 1. Arrange Value Objects
    ReferenceFrame refFrame(
        ReferenceType::Historical, 
        "Comparison against 2024 baseline", 
        "TRAJ-2024-BASE", 
        "2024-2025"
    );

    StructuralDeviation structDev(0.15, -0.05, 0.02, "Moderate Fragmentation");
    
    TemporalDeviationPattern tempPattern(
        DeviationTrend::Divergent, 
        -0.1, 
        0.05, 
        "Increasing volatility observed"
    );

    // 2. Act: Create Entity
    TrajectoryImpactProfile profile(
        "PROFILE-001",
        "TRAJ-2025-OBS",
        refFrame,
        structDev,
        tempPattern
    );

    // 3. Assert: Verify Data Integrity
    EXPECT_EQ(profile.getId(), "PROFILE-001");
    EXPECT_EQ(profile.getObservedTrajectoryId(), "TRAJ-2025-OBS");
    
    // Evaluate Deep Equality of Value Objects
    EXPECT_EQ(profile.getReference().type, ReferenceType::Historical);
    EXPECT_EQ(profile.getReference().description, "Comparison against 2024 baseline");
    
    EXPECT_DOUBLE_EQ(profile.getStructuralDeviation().spatialCoherenceDelta, 0.15);
    EXPECT_EQ(profile.getStructuralDeviation().semanticTag, "Moderate Fragmentation");

    EXPECT_EQ(profile.getTemporalPattern().trend, DeviationTrend::Divergent);
}

