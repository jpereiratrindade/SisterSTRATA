#pragma once

#include "core/value_objects/Vector3.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Core::Domain {

/**
 * @brief Represents a generic set of points in the world.
 * Can be a point cloud, a mesh, or a polyline.
 */
struct WorldEntity {
    std::string id;
    std::string type; // "point_cloud", "mesh", "polyline"
    std::vector<Core::ValueObjects::Vector3> points;
    std::vector<glm::vec3> colors;
    
    // Metadata can be added here (e.g., source file, creation time)
};

/**
 * @brief The aggregate state of the scientific world.
 * Holds all observable entities that need to be visualized or analyzed.
 */
struct WorldState {
    std::vector<WorldEntity> entities;
    
    void clear() {
        entities.clear();
    }
    
    void addEntity(const WorldEntity& entity) {
        entities.push_back(entity);
    }
};

} // namespace Core::Domain
