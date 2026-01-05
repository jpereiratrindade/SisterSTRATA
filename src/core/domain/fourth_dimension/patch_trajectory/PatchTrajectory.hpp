#pragma once

#include "PatchState.hpp"
#include <vector>
#include <memory>

namespace Core::Domain::FourthDimension::PatchTrajectory {

/**
 * @brief Aggregate Root for a Patch's history across time.
 * Follows Section 5 and 6 of DDD_PatchTrajectory_Analysis.
 */
class PatchTrajectory {
public:
    PatchTrajectory(int patchId) : patchId_(patchId) {}

    void addState(const PatchState& state) {
        history_.push_back(state);
    }

    int getPatchId() const { return patchId_; }
    const std::vector<PatchState>& getHistory() const { return history_; }

    // 5.1 Existência e Persistência
    int getLifespan() const { return static_cast<int>(history_.size()); }
    
    // 5.2 Processos Dominantes (Placeholders para análise espacial comparativa)
    int getSplitCount() const { return 0; } 
    int getMergeCount() const { return 0; }

    // 5.3 Dinâmica e Tendência
    float getNetAreaTrend() const {
        if (history_.size() < 2) return 0.0f;
        return history_.back().area - history_.front().area;
    }

    float getShapeVolatility() const {
        if (history_.size() < 2) return 0.0f;
        float totalVar = 0.0f;
        for (size_t i = 1; i < history_.size(); ++i) {
            totalVar += std::abs(history_[i].shapeIndex - history_[i-1].shapeIndex);
        }
        return totalVar / static_cast<float>(history_.size());
    }

    float getStructuralStabilityIndex() const {
        float volatility = getShapeVolatility();
        return 1.0f / (1.0f + volatility);
    }

    bool isIncreasing() const { return getNetAreaTrend() > 0.01f; }
    bool isDecreasing() const { return getNetAreaTrend() < -0.01f; }
    bool isStable() const { return std::abs(getNetAreaTrend()) <= 0.01f; }

private:
    int patchId_;
    std::vector<PatchState> history_;
};

} // namespace Core::Domain::FourthDimension::PatchTrajectory
