#pragma once

#include <vector>
#include <string>
#include <ctime>

namespace Core::Domain::FourthDimension {

/**
 * @brief Represents an immutable snapshot of the STRATA landscape state.
 * Captures the realized ecological cover and hydrology at a specific ordinal time.
 * This entity is OBSERVATIONAL and READ-ONLY.
 */
class TimeSlice {
public:
    TimeSlice(int id, int ordinal, const std::vector<int>& cover, const std::vector<bool>& water, const std::string& meta)
        : id_(id), ordinalIndex_(ordinal), ecologicalCoverState_(cover), waterMask_(water), metadata_(meta) {
        timestamp_ = std::time(nullptr);
    }

    // Identity
    int getId() const { return id_; }
    
    // Timeline
    int getOrdinalIndex() const { return ordinalIndex_; } // Ecological Time (Sequence)
    time_t getTimestamp() const { return timestamp_; }    // Metadata Time (System Clock)

    // State Data (Immutable)
    const std::vector<int>& getEcologicalCoverState() const { return ecologicalCoverState_; }
    const std::vector<bool>& getWaterMask() const { return waterMask_; }
    const std::string& getMetadata() const { return metadata_; }

private:
    int id_;
    int ordinalIndex_;
    time_t timestamp_;
    
    std::vector<int> ecologicalCoverState_; // Stores VegetationCode values
    std::vector<bool> waterMask_;           // Stores Hydrology mask
    
    std::string metadata_;
};

} // namespace Core::Domain::FourthDimension
