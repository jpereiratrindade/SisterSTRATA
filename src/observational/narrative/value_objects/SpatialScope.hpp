#pragma once

#include <optional>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing the spatial anchor of a narrative observation.
 * 
 * Determines "where" the narrative takes place or applies to.
 * Can be a specific 3D point, a discrete patch (cell), or a broader region.
 */
class SpatialScope {
public:
    enum class ScopeType {
        NONE,
        POINT,          // Specific x,y,z coordinate
        PATCH_ID,       // Specific cell/patch index in the grid
        REGION_BOX      // A bounding box (not fully implemented yet, placeholder)
    };

    struct Coordinates {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    SpatialScope() : m_type(ScopeType::NONE) {}

    // Constructor for Patch ID
    explicit SpatialScope(int patchId) 
        : m_type(ScopeType::PATCH_ID), m_patchId(patchId) {}

    // Constructor for Point
    explicit SpatialScope(float x, float y, float z) 
        : m_type(ScopeType::POINT), m_coordinates({x, y, z}) {}

    ScopeType getType() const { return m_type; }
    std::optional<int> getPatchId() const { return m_patchId; }
    std::optional<Coordinates> getCoordinates() const { return m_coordinates; }

    bool operator==(const SpatialScope& other) const {
        if (m_type != other.m_type) return false;
        if (m_patchId != other.m_patchId) return false;
        if (m_coordinates.has_value() != other.m_coordinates.has_value()) return false;
        if (m_coordinates.has_value()) {
            auto& a = m_coordinates.value();
            auto& b = other.m_coordinates.value();
            return a.x == b.x && a.y == b.y && a.z == b.z;
        }
        return true;
    }

    friend void to_json(nlohmann::json& j, const SpatialScope& obj) {
        j = nlohmann::json{
            {"type", static_cast<int>(obj.m_type)}
        };
        if (obj.m_patchId.has_value()) {
            j["patchId"] = obj.m_patchId.value();
        }
        if (obj.m_coordinates.has_value()) {
            auto& c = obj.m_coordinates.value();
            j["coordinates"] = { {"x", c.x}, {"y", c.y}, {"z", c.z} };
        }
    }

    friend void from_json(const nlohmann::json& j, SpatialScope& obj) {
        if (j.contains("type")) {
            obj.m_type = static_cast<ScopeType>(j.at("type").get<int>());
        }
        if (j.contains("patchId")) {
            obj.m_patchId = j.at("patchId").get<int>();
        }
        if (j.contains("coordinates")) {
            auto& c = j.at("coordinates");
            obj.m_coordinates = Coordinates{
                c.at("x").get<float>(),
                c.at("y").get<float>(),
                c.at("z").get<float>()
            };
        }
    }

private:
    ScopeType m_type;
    std::optional<int> m_patchId;
    std::optional<Coordinates> m_coordinates;
};

} // namespace SisterSTRATA::Observational::Narrative
