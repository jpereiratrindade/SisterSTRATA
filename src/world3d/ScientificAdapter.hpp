#pragma once

#include "core/value_objects/Vector3.hpp"
#include "world3d/rendering/Vertex.hpp"
#include <vector>

namespace World3D {

class ScientificAdapter {
public:
    static std::vector<Rendering::Vertex> convert(
        const std::vector<Core::ValueObjects::Vector3>& points, 
        const Core::ValueObjects::Vector3& origin
    );
};

} // namespace World3D
