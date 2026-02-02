#pragma once

#include "core/value_objects/TerrainVertex.hpp"
#include "world3d/rendering/Vertex.hpp"
#include <vector>

namespace World3D::Adapters {

inline Core::ValueObjects::TerrainVertex toTerrainVertex(const World3D::Rendering::Vertex& v) {
    return Core::ValueObjects::TerrainVertex{v.pos, v.color, v.normal, v.uv};
}

inline std::vector<Core::ValueObjects::TerrainVertex> toTerrainVertices(
    const std::vector<World3D::Rendering::Vertex>& vertices
) {
    std::vector<Core::ValueObjects::TerrainVertex> out;
    out.reserve(vertices.size());
    for (const auto& v : vertices) {
        out.push_back(toTerrainVertex(v));
    }
    return out;
}

inline void applyTerrainVertices(
    const std::vector<Core::ValueObjects::TerrainVertex>& src,
    std::vector<World3D::Rendering::Vertex>& dst
) {
    if (src.size() != dst.size()) return;
    for (size_t i = 0; i < src.size(); ++i) {
        dst[i].pos = src[i].pos;
        dst[i].color = src[i].color;
        dst[i].normal = src[i].normal;
        dst[i].uv = src[i].uv;
    }
}

inline void applyTerrainColors(
    const std::vector<Core::ValueObjects::TerrainVertex>& src,
    std::vector<World3D::Rendering::Vertex>& dst
) {
    if (src.size() != dst.size()) return;
    for (size_t i = 0; i < src.size(); ++i) {
        dst[i].color = src[i].color;
    }
}

} // namespace World3D::Adapters
