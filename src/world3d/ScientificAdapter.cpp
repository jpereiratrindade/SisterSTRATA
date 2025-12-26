#include "world3d/ScientificAdapter.hpp"

namespace World3D {

std::vector<Rendering::Vertex> ScientificAdapter::convert(
    const std::vector<Core::ValueObjects::Vector3>& points, 
    const Core::ValueObjects::Vector3& origin
) {
    std::vector<Rendering::Vertex> vertices;
    vertices.reserve(points.size());

    for (const auto& p : points) {
        Rendering::Vertex v;
        
        // Floating Origin Logic: Subtract origin (double) key, then cast to float
        v.pos.x = static_cast<float>(p.x - origin.x);
        v.pos.y = static_cast<float>(p.y - origin.y);
        v.pos.z = static_cast<float>(p.z - origin.z);

        // Simple coloring based on height (relative Z)
        // Normalize Z somewhat for coloring (assuming range -10 to 10 for demo)
        float normalizedZ = (v.pos.z + 10.0f) / 20.0f;
        v.color = {normalizedZ, 0.2f, 1.0f - normalizedZ}; // Blue to Red gradient

        vertices.push_back(v);
    }

    return vertices;
}

} // namespace World3D
