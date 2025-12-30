#pragma once

#include <string>
#include <vector>

namespace Core::Domain::Shared::ValueObjects {

/**
 * @brief Represents the degree of internal consistency of a territorial configuration.
 * 
 * In the STRATA philosophy, coherence is not a judgment of "good" or "bad" normative use,
 * but a measure of how well the Land Use Hypotheses align with the bio-physical reality
 * and spatial structural patterns.
 */
class CoherenceScore {
public:
    /**
     * @brief Constructs a new Coherence Score.
     * @param value The numerical score (0.0 to 1.0).
     * @param description A qualitative description of the score (e.g., "High Tension", "Coherent").
     */
    CoherenceScore(float value, std::string description) 
        : value_(value), description_(std::move(description)) {}

    /**
     * @brief Gets the numerical coherence value.
     * @return float between 0.0 (Chaotic/Incoherent) and 1.0 (Highly Coherent).
     */
    float getValue() const { return value_; }

    /**
     * @brief Gets the qualitative description of the coherence state.
     */
    const std::string& getDescription() const { return description_; }

    /**
     * @brief Checks if the territory is effectively coherent above a threshold.
     */
    bool isCoherent(float threshold = 0.7f) const { return value_ >= threshold; }

private:
    float value_;
    std::string description_;
};

} // namespace Core::Domain::Shared::ValueObjects
