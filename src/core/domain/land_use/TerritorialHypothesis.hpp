#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "core/domain/land_use/LandUsePotential.hpp"

namespace Core::Domain::LandUse {

enum class HypothesisType {
    Exploratory,
    Scenario,
    ConstraintTest
};

enum class AllocationMethod {
    GlobalConstraint,
    PatchSelection,
    ManualZone
};

/**
 * @brief Represents a single Allocation Rule within a hypothesis.
 * 
 * Rules define HOW a Land Use Potential is proposed for a specific location or context.
 * They are not deterministic assignments but prioritized candidates.
 */
struct AllocationRule {
    int priority; // 1 = Highest
    std::string landUseId;
    AllocationMethod method;
    
    // Generic parameters map for flexibility (SlopeRange, SoilOrders, etc.)
    // In a full implementation, this might be a polymorphic Strategy pattern or a robust configuration object.
    std::map<std::string, std::string> parameters; 
};

/**
 * @brief Entity representing a Territorial Hypothesis.
 * 
 * A hypothesis is a coherent set of Land Use Potentials and Allocation Rules
 * that proposes a candidate configuration for the territory.
 * It is the primary input for the Coherence Evaluation process.
 */
class TerritorialHypothesis {
public:
    using ID = std::string;

    TerritorialHypothesis(ID id, std::string name, HypothesisType type)
        : id_(std::move(id)), name_(std::move(name)), type_(type) {}

    const ID& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    HypothesisType getType() const { return type_; }
    
    const std::vector<LandUsePotential>& getLandUseTypes() const { return landUseTypes_; }
    const std::vector<AllocationRule>& getAllocationRules() const { return rules_; }

    void addLandUseType(LandUsePotential potential) {
        landUseTypes_.push_back(std::move(potential));
    }

    void addAllocationRule(AllocationRule rule) {
        rules_.push_back(std::move(rule));
    }

    void updateLandUseType(LandUsePotential potential) {
        for (auto& existing : landUseTypes_) {
            if (existing.getId() == potential.getId()) {
                existing = std::move(potential);
                return;
            }
        }
        // If not found, maybe add it? For now, do nothing or log.
    }

    void removeAllocationRule(size_t index) {
        if (index < rules_.size()) {
            rules_.erase(rules_.begin() + index);
        }
    }

private:
    ID id_;
    std::string name_;
    HypothesisType type_;
    
    std::vector<LandUsePotential> landUseTypes_;
    std::vector<AllocationRule> rules_;
};

} // namespace Core::Domain::LandUse
