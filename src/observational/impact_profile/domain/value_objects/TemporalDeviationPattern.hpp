#pragma once

#include <string>

namespace SisterSTRATA::Observational::ImpactProfile::Domain {

/**
 * @brief Describes the dynamic behavior of the deviation over time.
 */
enum class DeviationTrend {
    Convergent,   // The observed trajectory is moving closer to the reference
    Divergent,    // The observed trajectory is moving away from the reference
    Parallel,     // The deviation remains constant
    Erratic       // No clear pattern
};

/**
 * @brief Value Object representing the temporal aspect of the deviation.
 */
struct TemporalDeviationPattern {
    DeviationTrend trend;
    double stabilityShift;   // Change in stability metric
    double volatilityTrend;  // Change in volatility (e.g., more erratic behavior)
    
    std::string description;

    TemporalDeviationPattern(DeviationTrend tr, double stab, double vol, std::string desc)
        : trend(tr), stabilityShift(stab), volatilityTrend(vol), description(std::move(desc)) {}

    bool operator==(const TemporalDeviationPattern& other) const {
        return trend == other.trend &&
               stabilityShift == other.stabilityShift &&
               volatilityTrend == other.volatilityTrend &&
               description == other.description;
    }
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Domain
