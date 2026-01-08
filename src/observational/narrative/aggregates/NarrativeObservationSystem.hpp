#pragma once

#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include "src/observational/narrative/entities/NarrativeState.hpp"

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Aggregate Root for the Narrative Observation Context.
 * 
 * Acts as a passive container for narrative observations.
 * 
 * CONTRACT:
 * - Append-only: No deletion or modification of existing states.
 * - No Semantic Validation: It accepts observations as valid declarations of a source.
 * - No Causal Inference: Does not link states implies causation.
 */
class NarrativeObservationSystem {
public:
    NarrativeObservationSystem() = default;

    /**
     * @brief Registers a new narrative observation.
     * 
     * @param state The immutable state to register.
     * @throws std::runtime_error if a state with the same ID already exists (integrity check).
     */
    void registerObservation(std::shared_ptr<NarrativeState> state) {
        if (!state) {
            throw std::invalid_argument("Cannot register null state.");
        }

        // Integrity check: distinct IDs
        for (const auto& existing : m_observations) {
            if (existing->getId() == state->getId()) {
                throw std::runtime_error("Duplicate NarrativeStateID: " + state->getId());
            }
        }

        m_observations.push_back(state);
    }

    /**
     * @brief Retrieves the full history of observations.
     * 
     * @return const reference to the vector of states.
     */
    const std::vector<std::shared_ptr<NarrativeState>>& getObservations() const {
        return m_observations;
    }

    // Explicitly disabling update/delete functionality to enforce append-only contract.
    void removeObservation(const std::string&) = delete;
    void updateObservation(const std::string&, const NarrativeState&) = delete;

private:
    std::vector<std::shared_ptr<NarrativeState>> m_observations;
};

} // namespace SisterSTRATA::Observational::Narrative
