#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/soils/SoilSystem.hpp"

namespace Core::Domain::Territory {

/**
 * @brief Immutable Value Object representing a snapshot of the territory state.
 * 
 * Captures the complete biophysical state at a specific moment in the simulation trajectory.
 * Used for resilience analysis, time-series comparison, and auditability.
 * 
 * @warning TerritorySnapshot represents a provisional state under a specific hypothesis 
 *          and must never be interpreted as a final or optimal configuration.
 */
class TerritorySnapshot {
public:
    using TimePoint = std::chrono::system_clock::time_point;

    TerritorySnapshot(
        size_t eventIndex,
        TimePoint timestamp,
        std::string hypothesisId,
        Hydro::HydroGrid hydro,
        std::vector<Soils::SiBCSClassification> soils,
        std::vector<float> slopes,
        std::vector<std::string> landUse
    ) : eventIndex_(eventIndex),
        timestamp_(timestamp),
        hypothesisId_(std::move(hypothesisId)),
        hydroState_(std::move(hydro)),
        soilState_(std::move(soils)),
        slopeState_(std::move(slopes)),
        landUseState_(std::move(landUse))
    {}

    // Accessors (Immutable)
    size_t getEventIndex() const { return eventIndex_; }
    TimePoint getTimestamp() const { return timestamp_; }
    const std::string& getHypothesisId() const { return hypothesisId_; }
    const Hydro::HydroGrid& getHydroState() const { return hydroState_; }
    const std::vector<Soils::SiBCSClassification>& getSoilState() const { return soilState_; }
    const std::vector<float>& getSlopeState() const { return slopeState_; }
    const std::vector<std::string>& getLandUseState() const { return landUseState_; }

private:
    size_t eventIndex_; // Explicit order of events
    TimePoint timestamp_;
    std::string hypothesisId_;
    
    // Captured State
    Hydro::HydroGrid hydroState_;
    std::vector<Soils::SiBCSClassification> soilState_;
    std::vector<float> slopeState_;
    std::vector<std::string> landUseState_; // Added for Resilience Overlap Analysis
};

} // namespace Core::Domain::Territory
