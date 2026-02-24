#pragma once

#include "observational/interpretation/entities/InterpretationSnapshot.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Interpretation {

/**
 * @brief Repository for managing persistent AI interpretations.
 */
class InterpretationRepository {
public:
    InterpretationRepository() = default;

    void addSnapshot(const InterpretationSnapshot& snapshot) {
        m_snapshots.push_back(snapshot);
    }

    const std::vector<InterpretationSnapshot>& getSnapshots() const {
        return m_snapshots;
    }

    void serialize(const std::string& filepath) const {
        nlohmann::json j;
        j["interpretations"] = m_snapshots;
        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
        }
    }

    void deserialize(const std::string& filepath) {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) return;
        nlohmann::json j;
        try {
            ifs >> j;
            if (j.contains("interpretations")) {
                m_snapshots = j.at("interpretations").get<std::vector<InterpretationSnapshot>>();
            }
        } catch (const std::exception& e) {
            std::cerr << "[InterpretationRepository] Failed to deserialize "
                      << filepath << ": " << e.what() << std::endl;
        }
    }

private:
    std::vector<InterpretationSnapshot> m_snapshots;
};

} // namespace SisterSTRATA::Observational::Interpretation
