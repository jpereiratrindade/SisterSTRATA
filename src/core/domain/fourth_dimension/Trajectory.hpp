#pragma once

#include "TimeSlice.hpp"
#include <vector>

namespace Core::Domain::FourthDimension {

/**
 * @brief Aggregate Root for the Fourth Dimension System.
 * Manages the ordered sequence of TimeSlices (Trajectory).
 */
class Trajectory {
public:
    Trajectory() = default;

    /**
     * @brief Adds a new time slice to the trajectory.
     * @param slice The immutable snapshot to append.
     */
    void addTimeSlice(const TimeSlice& slice) {
        slices_.push_back(slice);
    }

    /**
     * @brief returns the next available ordinal index.
     * Starts at 1.
     */
    int getNextOrdinal() const {
        if (slices_.empty()) return 1;
        return slices_.back().getOrdinalIndex() + 1;
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
