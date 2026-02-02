#pragma once

#include <glm/glm.hpp>

namespace Core::ValueObjects {

struct TerrainVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

} // namespace Core::ValueObjects
