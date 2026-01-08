#pragma once

#include "src/observational/narrative/entities/NarrativeState.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory> 
#include <fstream>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Aggregate Root for the Narrative Observation Bounded Context.
 */
class NarrativeObservationSystem {
public:
    NarrativeObservationSystem() = default;

    /**
     * @brief Registers a new observation into the system.
     */
    void registerObservation(const NarrativeState& observation) {
        // Enforce Invariant: Structural Uniqueness
        auto itId = std::find_if(m_history.begin(), m_history.end(),
            [&](const NarrativeState& existing) {
                return existing.getId() == observation.getId();
            });

        if (itId != m_history.end()) {
            throw std::invalid_argument("Duplicate NarrativeStateID: " + observation.getId());
        }

        auto itContent = std::find_if(m_history.begin(), m_history.end(),
            [&](const NarrativeState& existing) {
                bool sameSource = existing.getSource().getSourceId() == observation.getSource().getSourceId();
                bool sameTime = existing.getTemporalContext().getLabel() == observation.getTemporalContext().getLabel();
                return sameSource && sameTime;
            });

        if (itContent != m_history.end()) {
            throw std::invalid_argument(
                "Duplicate Observation: Source '" + observation.getSource().getSourceId() + 
                "' already has an entry for time '" + observation.getTemporalContext().getLabel() + "'"
            );
        }

        m_history.push_back(observation);
    }

    const std::vector<NarrativeState>& getHistory() const {
        return m_history;
    }

    // Persistence Logic
    void serialize(const std::string& filepath) const {
        nlohmann::json j;
        j["history"] = m_history;
        
        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
        } else {
            throw std::runtime_error("Failed to open file for writing narrative data: " + filepath);
        }
    }

    void deserialize(const std::string& filepath) {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            // It's acceptable if the file doesn't exist yet (new project), but we should warn or handle.
            // For now, if file missing, we assume empty or throw. 
            // Better behavior: clear history? Or just return if file not found?
            // "Replace current state" implies clear.
            // If file doesn't exist, maybe do nothing (or clear to match 'empty file').
            return; 
        }

        nlohmann::json j;
        try {
            ifs >> j;
            if (j.contains("history")) {
                m_history = j["history"].get<std::vector<NarrativeState>>();
            }
        } catch (const nlohmann::json::parse_error& e) {
            // Corrupt file or empty
             throw std::runtime_error("Failed to parse narrative data JSON: " + std::string(e.what()));
        }
    }

    void clear() {
        m_history.clear();
    }

    void removeObservation(const std::string& id) {
        auto it = std::remove_if(m_history.begin(), m_history.end(),
            [&](const NarrativeState& obs) {
                return obs.getId() == id;
            });

        if (it == m_history.end()) {
            throw std::runtime_error("Narrative Observation not found: " + id);
        }

        m_history.erase(it, m_history.end());
    }

    void updateObservation(const std::string& id, const NarrativeState& newObservation) {
        auto it = std::find_if(m_history.begin(), m_history.end(),
            [&](const NarrativeState& obs) {
                return obs.getId() == id;
            });

        if (it == m_history.end()) {
             throw std::runtime_error("Narrative Observation not found: " + id);
        }

        if (newObservation.getId() != id) {
             auto itCollision = std::find_if(m_history.begin(), m_history.end(),
                [&](const NarrativeState& existing) {
                    return existing.getId() == newObservation.getId() && existing.getId() != id;
                });
            
            if (itCollision != m_history.end()) {
                 throw std::invalid_argument("Cannot update observation: New ID " + newObservation.getId() + " is already in use.");
            }
        }

        *it = newObservation;
    }

private:
    std::vector<NarrativeState> m_history;
};

} // namespace SisterSTRATA::Observational::Narrative
