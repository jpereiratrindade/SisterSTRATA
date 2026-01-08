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

    void removeSystem(const std::string&) = delete;
    void updateSystem(const std::string&, const DiscursiveSystem&) = delete;

private:
    std::vector<DiscursiveSystem> m_systems;
};

} // namespace SisterSTRATA::Observational::Discursive
