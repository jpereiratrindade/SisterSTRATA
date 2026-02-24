#pragma once

#include "observational/recommendation/entities/RecommendationSnapshot.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Recommendation {

using RecommendationTrajectoryID = std::string;

/**
 * @brief Aggregate Root for the Recommendation Trajectory Bounded Context.
 */
class RecommendationTrajectory {
public:
    RecommendationTrajectory() = default;

    RecommendationTrajectory(RecommendationTrajectoryID id, std::map<std::string, std::string> metadata)
        : m_id(std::move(id)), m_metadata(std::move(metadata)) {}

    const RecommendationTrajectoryID& getId() const { return m_id; }
    const std::vector<RecommendationSnapshot>& getSnapshots() const { return m_snapshots; }
    const std::map<std::string, std::string>& getMetadata() const { return m_metadata; }

    void addSnapshot(const RecommendationSnapshot& snapshot) {
        auto itId = std::find_if(m_snapshots.begin(), m_snapshots.end(),
            [&](const RecommendationSnapshot& existing) {
                return existing.getId() == snapshot.getId();
            });

        if (itId != m_snapshots.end()) {
            throw std::invalid_argument("Duplicate RecommendationSnapshotID: " + snapshot.getId());
        }

        m_snapshots.push_back(snapshot);
    }

    void serialize(const std::string& filepath) const {
        nlohmann::json j;
        j["id"] = m_id;
        j["metadata"] = m_metadata;
        j["snapshots"] = m_snapshots;

        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << j.dump(4);
        } else {
            throw std::runtime_error("Failed to open file for writing recommendation trajectory: " + filepath);
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
            if (j.contains("id")) {
                m_id = j.at("id").get<RecommendationTrajectoryID>();
            }
            if (j.contains("metadata")) {
                m_metadata = j.at("metadata").get<std::map<std::string, std::string>>();
            }
            if (j.contains("snapshots")) {
                m_snapshots = j.at("snapshots").get<std::vector<RecommendationSnapshot>>();
            }
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("Failed to parse recommendation trajectory JSON: " + std::string(e.what()));
        }
    }

    void clear() {
        m_snapshots.clear();
        m_metadata.clear();
        m_id.clear();
    }

    void removeSnapshot(const std::string& id) {
        auto it = std::remove_if(m_snapshots.begin(), m_snapshots.end(),
            [&](const RecommendationSnapshot& s) { return s.getId() == id; });
        
        if (it != m_snapshots.end()) {
            m_snapshots.erase(it, m_snapshots.end());
        }
    }

    void updateSnapshot(const std::string& id, const RecommendationSnapshot& newSnapshot) {
        auto it = std::find_if(m_snapshots.begin(), m_snapshots.end(),
            [&](const RecommendationSnapshot& s) { return s.getId() == id; });

        if (it != m_snapshots.end()) {
            // Check if ID changed and conflicts
            if (newSnapshot.getId() != id) {
                 auto collision = std::find_if(m_snapshots.begin(), m_snapshots.end(),
                    [&](const RecommendationSnapshot& s) { return s.getId() == newSnapshot.getId(); });
                 if (collision != m_snapshots.end()) {
                     throw std::invalid_argument("Cannot update: New ID already exists.");
                 }
            }
            *it = newSnapshot;
        } else {
             throw std::invalid_argument("Snapshot not found: " + id);
        }
    }

private:
    RecommendationTrajectoryID m_id;
    std::vector<RecommendationSnapshot> m_snapshots;
    std::map<std::string, std::string> m_metadata;
};

} // namespace SisterSTRATA::Observational::Recommendation
