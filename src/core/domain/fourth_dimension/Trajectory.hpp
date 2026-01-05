#pragma once

#include "TimeSlice.hpp"
#include "TrajectoryPersistenceService.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace Core::Domain::FourthDimension {

/**
 * @brief Aggregate Root for the Fourth Dimension System.
 * Manages the ordered sequence of TimeSlices (Trajectory).
 * Implements LOD Temporal (persistence) to manage RAM.
 */
class Trajectory {
public:
    Trajectory() = default;

    /**
     * @brief Adds a new time slice and enforces RAM limits.
     */
    void addTimeSlice(const TimeSlice& slice) {
        slices_.push_back(slice);
        enforceRamLimit();
    }

    /**
     * @brief Offloads older slices to disk if the count exceeds limit.
     */
    void enforceRamLimit(size_t maxInRam = 5, const std::string& cacheDir = ".") {
        if (slices_.size() <= maxInRam) return;

        // Keep the most recent 'maxInRam' slices in RAM
        for (size_t i = 0; i < slices_.size() - maxInRam; ++i) {
            if (!slices_[i].isProxy()) {
                if (TrajectoryPersistenceService::saveToDisk(slices_[i], cacheDir)) {
                    std::cout << "[Trajectory] Offloaded slice " << slices_[i].getOrdinalIndex() << " to disk (LOD Temporal).\n";
                }
            }
        }
    }

    /**
     * @brief returns the next available ordinal index.
     * Starts at 1.
     */
    int getNextOrdinal() const {
        if (slices_.empty()) return 1;
        return slices_.back().getOrdinalIndex() + 1;
    }

    std::vector<TimeSlice>& getTimeSlices() {
        return slices_;
    }

    const std::vector<TimeSlice>& getTimeSlices() const {
        return slices_;
    }

    const TimeSlice* getLatest() const {
        if (slices_.empty()) return nullptr;
        return &slices_.back();
    }
    
    void clear() {
        slices_.clear();
    }

private:
    std::vector<TimeSlice> slices_;
};

} // namespace Core::Domain::FourthDimension
