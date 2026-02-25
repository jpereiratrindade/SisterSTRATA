#include <vector>

#include <gtest/gtest.h>

#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/domain/fourth_dimension/TrajectoryPersistenceService.hpp"

using namespace Core::Domain::FourthDimension;

TEST(CoreTrajectoryLODTest, OffloadsOlderSlicesAndCanReloadProxy) {
    Trajectory traj;

    for (int i = 1; i <= 10; ++i) {
        std::vector<int> cover(100, i);
        std::vector<bool> water(100, (i % 2 == 0));
        TimeSlice slice(i, i, cover, water, "Metadata " + std::to_string(i));
        EXPECT_EQ(slice.getTimestamp(), static_cast<time_t>(i));
        traj.addTimeSlice(slice);
    }

    auto& slices = traj.getTimeSlices();
    ASSERT_EQ(slices.size(), 10u);

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(slices[i].isProxy());
        EXPECT_TRUE(slices[i].getEcologicalCoverState().empty());
    }

    for (int i = 5; i < 10; ++i) {
        EXPECT_FALSE(slices[i].isProxy());
        EXPECT_FALSE(slices[i].getEcologicalCoverState().empty());
    }

    const bool success = TrajectoryPersistenceService::loadFromDisk(slices[0]);
    EXPECT_TRUE(success);
    EXPECT_FALSE(slices[0].isProxy());
    ASSERT_EQ(slices[0].getEcologicalCoverState().size(), 100u);
    EXPECT_EQ(slices[0].getEcologicalCoverState()[0], 1);
}
