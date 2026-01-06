#pragma once

#include <vector>
#include <string>
#include <ctime>

namespace Core::Domain::FourthDimension {

enum class ClassificationType {
    ScenarioIndex,
    SemanticCode
};

/**
 * @brief Represents an immutable snapshot of the STRATA landscape state.
 * Captures the realized ecological cover and hydrology at a specific ordinal time.
 * This entity is OBSERVATIONAL and READ-ONLY.
 */
class TimeSlice {
public:
    TimeSlice(int id, int ordinal, const std::vector<int>& cover, const std::vector<bool>& water, const std::string& meta, ClassificationType type = ClassificationType::ScenarioIndex)
        : id_(id), ordinalIndex_(ordinal), ecologicalCoverState_(cover), waterMask_(water), metadata_(meta), type_(type) {
        timestamp_ = std::time(nullptr);
    }

    // Identity
    int getId() const { return id_; }
    
    // Timeline
    int getOrdinalIndex() const { return ordinalIndex_; } // Ecological Time (Sequence)
    time_t getTimestamp() const { return timestamp_; }    // Metadata Time (System Clock)

    // State Data (Immutable-ish, can be reloaded)
    const std::vector<int>& getEcologicalCoverState() const { return ecologicalCoverState_; }
    const std::vector<bool>& getWaterMask() const { return waterMask_; }
    const std::string& getMetadata() const { return metadata_; }
    ClassificationType getClassificationType() const { return type_; }

    // LOD Temporal Support
    bool isProxy() const { return isProxy_; }
    const std::string& getDiskPath() const { return diskPath_; }
    
    void setDiskPath(const std::string& path) { 
        diskPath_ = path; 
        isProxy_ = !path.empty();
    }

    void unload() {
        ecologicalCoverState_.clear();
        ecologicalCoverState_.shrink_to_fit();
        waterMask_.clear();
        waterMask_.shrink_to_fit();
        isProxy_ = true;
    }

    void load(const std::vector<int>& cover, const std::vector<bool>& water) {
        ecologicalCoverState_ = cover;
        waterMask_ = water;
        isProxy_ = false;
    }

private:
    int id_;
    int ordinalIndex_;
    time_t timestamp_;
    
    std::vector<int> ecologicalCoverState_; 
    std::vector<bool> waterMask_;           
    
    std::string metadata_;
    ClassificationType type_ = ClassificationType::ScenarioIndex;

    bool isProxy_ = false;
    std::string diskPath_;
};

} // namespace Core::Domain::FourthDimension
