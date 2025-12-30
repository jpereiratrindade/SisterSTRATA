#pragma once

#include "core/domain/land_use/LandUsePotential.hpp"
#include "core/domain/soils/SiBCS.hpp"
#include <string>

namespace Core::Domain::LandUse {

/**
 * @brief Result of a compatibility evaluation.
 */
enum class CompatibilityStatus {
    Compatible,   ///< Fully supported by bio-physical conditions.
    Borderline,   ///< Supported but with tension/risks.
    Incoherent    ///< Bio-physically unsustainable or impossible.
};

/**
 * @brief Detailed result of compatibility check.
 */
struct CompatibilityResult {
    CompatibilityStatus status;
    std::string reason;

    bool isCoherent() const { return status != CompatibilityStatus::Incoherent; }
};

/**
 * @brief Domain Service responsible for evaluating the plausibility of a Land Use Hypothesis.
 * 
 * "The Guardian of Plausibility". This service enforces the bio-physical rules of the STRATA world.
 * It does not decide what happens, but it judges the coherence of what the user proposes.
 */
class LandUseCompatibilityService {
public:
    /**
     * @brief Evaluates if a specific Land Use is compatible with a given soil profile and slope.
     * 
     * @param landUse The hypothesis to test.
     * @param soil The soil classification at the location.
     * @param slopeDegrees The local slope.
     * @return CompatibilityResult The judgment of the simulation engine.
     */
    static CompatibilityResult evaluate(const LandUsePotential& landUse, 
                                      const Soils::SiBCSClassification& soil, 
                                      float slopeDegrees) {
        // Placeholder Logic for v0.1
        // In reality, this would query a dense ruleset or matrix.
        
        if (landUse.getId() == "Agriculture_Intensive") {
            if (slopeDegrees > 15.0f) {
                return {CompatibilityStatus::Incoherent, "Slope too steep for intensive machinery (>15 deg)."};
            }
            if (soil.Order == Soils::SoilOrder::Neossolos && soil.SubOrder == Soils::SoilSubOrder::Litolicos) {
                return {CompatibilityStatus::Incoherent, "Soil too shallow (Neossolo Litolico)."};
            }
        }
        
        if (landUse.getId() == "Conservation") {
            return {CompatibilityStatus::Compatible, "Conservation is generally compatible everywhere."};
        }

        return {CompatibilityStatus::Compatible, "No constraints found for this use."};
    }
};

} // namespace Core::Domain::LandUse
