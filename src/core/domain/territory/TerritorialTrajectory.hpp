#pragma once

#include <vector>
#include <optional>
#include "core/domain/territory/TerritorySnapshot.hpp"

namespace Core::Domain::Territory {

/**
 * @brief Aggregate Root for the Temporal Dimension of the Territory.
 * 
 * Manages the ordered sequence of TerritorySnapshots.
 * Enables resilience analysis by providing access to the history of biophysical states.
 */
class TerritorialTrajectory {
public:
    TerritorialTrajectory() = default;

    /**
     * @brief Appends a new state to the trajectory history.
     * @param snapshot The immutable snapshot to store.
     */
    void addSnapshot(TerritorySnapshot snapshot) {
        history_.push_back(std::move(snapshot));
    }

    /**
     * @brief Detailed access to a specific point in history.
     * @param index The event index to retrieve.
     * @return const TerritorySnapshot& Reference to the immutable snapshot.
     */
    const TerritorySnapshot& getSnapshotAt(size_t index) const {
        // Simple access, checking bounds logic should be in service or assert
        return history_.at(index);
    }

    /**
     * @brief Returns the most recent snapshot committed to history.
     */
    const TerritorySnapshot* getCurrentState() const {
        if (history_.empty()) return nullptr;
        return &history_.back();
    }

    /**
     * @brief Number of recorded states.
     */
    size_t size() const {
        return history_.size();
    }

    void clear() {
        history_.clear();
    }

private:
    std::vector<TerritorySnapshot> history_;
};

} // namespace Core::Domain::Territory
