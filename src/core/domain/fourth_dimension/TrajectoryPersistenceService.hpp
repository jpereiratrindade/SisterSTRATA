#pragma once

#include "TimeSlice.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>

namespace Core::Domain::FourthDimension {

class Trajectory;

/**
 * @brief Domain Service responsible for binary serialization of TimeSlices.
 * Enables LOD Temporal (Persistence) to save RAM.
 */
class TrajectoryPersistenceService {
public:
    static bool saveToDisk(TimeSlice& slice, const std::string& directory);
    static bool loadFromDisk(TimeSlice& slice);
    static bool saveTrajectory(Trajectory& trajectory, const std::string& directory, const std::string& manifestName);
    static bool loadTrajectory(Trajectory& trajectory, const std::string& directory, const std::string& manifestName);
};

} // namespace Core::Domain::FourthDimension
