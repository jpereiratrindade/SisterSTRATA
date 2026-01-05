#include <iostream>
#include <vector>
#include <cassert>
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/domain/fourth_dimension/TrajectoryPersistenceService.hpp"

using namespace Core::Domain::FourthDimension;

void testLOD() {
    Trajectory traj;
    std::cout << "Testing Trajectory LOD Temporal...\n";

    // Create 10 slices (Limit is 5 in RAM)
    for (int i = 1; i <= 10; ++i) {
        std::vector<int> cover(100, i); // Some data
        std::vector<bool> water(100, (i % 2 == 0));
        TimeSlice slice(i, i, cover, water, "Metadata " + std::to_string(i));
        traj.addTimeSlice(slice);
    }

    auto& slices = traj.getTimeSlices();
    assert(slices.size() == 10);

    // Indices 0 to 4 should be proxies (offloaded)
    // Indices 5 to 9 should be in RAM
    for (int i = 0; i < 5; ++i) {
        assert(slices[i].isProxy() == true);
        assert(slices[i].getEcologicalCoverState().empty());
        std::cout << "Slice " << slices[i].getOrdinalIndex() << " is proxy: OK\n";
    }

    for (int i = 5; i < 10; ++i) {
        assert(slices[i].isProxy() == false);
        assert(!slices[i].getEcologicalCoverState().empty());
        std::cout << "Slice " << slices[i].getOrdinalIndex() << " in RAM: OK\n";
    }

    // Now reload one proxy
    std::cout << "Reloading slice 1...\n";
    bool success = TrajectoryPersistenceService::loadFromDisk(slices[0]);
    assert(success);
    assert(slices[0].isProxy() == false);
    assert(slices[0].getEcologicalCoverState().size() == 100);
    assert(slices[0].getEcologicalCoverState()[0] == 1);
    std::cout << "Slice 1 reloaded correctly: OK\n";

    std::cout << "Trajectory LOD Temporal Test Passed!\n";
}

int main() {
    try {
        testLOD();
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
