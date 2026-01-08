#pragma once

#include "src/observational/discursive/entities/DiscursiveSystem.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Discursive {

/**
 * @brief Aggregate Root for the Discursive System Bounded Context.
 */
class DiscursiveSystemRepository {
public:
    DiscursiveSystemRepository() = default;

    /**
     * @brief Registers a new discursive system into the repository.
     */
    void registerSystem(const DiscursiveSystem& system) {
        auto itId = std::find_if(m_systems.begin(), m_systems.end(),
            [&](const DiscursiveSystem& existing) {
                return existing.getId() == system.getId();
            });

        if (itId != m_systems.end()) {
            throw std::invalid_argument("Duplicate DiscursiveSystemID: " + system.getId());
        }

        m_systems.push_back(system);
    }

    const std::vector<DiscursiveSystem>& getSystems() const {
        return m_systems;
    }

    void serialize(const std::string& filepath) const {
        nlohmann::json j;
        j["systems"] = m_systems;

        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
        } else {
            throw std::runtime_error("Failed to open file for writing discursive systems: " + filepath);
        }
    }

    void deserialize(const std::string& filepath) {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            return;
        }

        nlohmann::json j;
        try {
            ifs >> j;
            if (j.contains("systems")) {
                m_systems = j["systems"].get<std::vector<DiscursiveSystem>>();
            }
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("Failed to parse discursive systems JSON: " + std::string(e.what()));
        }
    }

    void clear() {
        m_systems.clear();
    }

    void removeSystem(const std::string& id) {
        auto it = std::remove_if(m_systems.begin(), m_systems.end(),
            [&](const DiscursiveSystem& system) {
                return system.getId() == id;
            });
        
        if (it == m_systems.end()) {
            throw std::runtime_error("Discursive System not found: " + id);
        }

        m_systems.erase(it, m_systems.end());
    }

    void updateSystem(const std::string& id, const DiscursiveSystem& newSystem) {
        auto it = std::find_if(m_systems.begin(), m_systems.end(),
            [&](const DiscursiveSystem& system) {
                return system.getId() == id;
            });

        if (it == m_systems.end()) {
            throw std::runtime_error("Discursive System not found: " + id);
        }

        // Preserve ID integrity if needed, or allow ID change? 
        // Usually ID should match. If newSystem has different ID, it might be a rename or error.
        // For simplicity, we assign the new system to the position.
        if (newSystem.getId() != id) {
             // Check if the new ID collides with another EXISTING system (that is NOT the one we are updating)
             auto itCollision = std::find_if(m_systems.begin(), m_systems.end(),
                [&](const DiscursiveSystem& existing) {
                    return existing.getId() == newSystem.getId() && existing.getId() != id;
                });
            
            if (itCollision != m_systems.end()) {
                 throw std::invalid_argument("Cannot update system: New ID " + newSystem.getId() + " is already in use.");
            }
        }

        *it = newSystem;
    }

private:
    std::vector<DiscursiveSystem> m_systems;
};

} // namespace SisterSTRATA::Observational::Discursive
