#pragma once

#include <string>

namespace SisterSTRATA::Observational::ImpactProfile::Domain {

/**
 * @brief Value Object representing *how* the structure of the landscape has deviated.
 * 
 * Captures metrics related to spatial configuration, fragmentation, and coherence.
 * All values are deltas (Observed - Reference).
 */
struct StructuralDeviation {
    double spatialCoherenceDelta;   // Change in spatial coherence
    double fragmentationIndexDelta; // Change in fragmentation level
    double areaTrendDelta;          // Difference in area trend slopes
    
    // Semantic interpretation of the deviation (e.g., "Highly Fragmented", "Structural Collapse")
    // Populated by the Analyzer based on thresholds.
    std::string semanticTag; 

    StructuralDeviation(double coherenceD, double fragD, double areaD, std::string tag = "")
        : spatialCoherenceDelta(coherenceD), 
          fragmentationIndexDelta(fragD), 
          areaTrendDelta(areaD),
          semanticTag(std::move(tag)) {}

    bool operator==(const StructuralDeviation& other) const {
        return spatialCoherenceDelta == other.spatialCoherenceDelta &&
               fragmentationIndexDelta == other.fragmentationIndexDelta &&
               areaTrendDelta == other.areaTrendDelta &&
               semanticTag == other.semanticTag;
    }
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Domain
