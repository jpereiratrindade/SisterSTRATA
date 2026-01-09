#pragma once

#include <string>
#include <optional>

namespace SisterSTRATA::Observational::ImpactProfile::Domain {

/**
 * @brief Defines the type of reference used for comparison.
 */
enum class ReferenceType {
    Historical,    // Comparing against a past state of the same system
    Simulated,     // Comparing against a simulation (e.g., VegetationSystemOriginal)
    Theoretical,   // Comparing against an ideal or theoretical model
    ControlGroup   // Comparing against a declared control scenario
};

/**
 * @brief Value Object that defines *against what* an observation is being compared.
 * 
 * Crucial for the "Relational" aspect of the analysis. An impact profile 
 * without a reference frame is meaningless.
 */
struct ReferenceFrame {
    ReferenceType type;
    std::string description;
    std::string referenceId; // ID of the referenced trajectory or system
    std::optional<std::string> timeWindowLabel;

    ReferenceFrame(ReferenceType t, std::string desc, std::string refId, std::optional<std::string> window = std::nullopt)
        : type(t), description(std::move(desc)), referenceId(std::move(refId)), timeWindowLabel(std::move(window)) {}

    // Equality operator for Value Object semantics
    bool operator==(const ReferenceFrame& other) const {
        return type == other.type && 
               description == other.description && 
               referenceId == other.referenceId &&
               timeWindowLabel == other.timeWindowLabel;
    }
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Domain
