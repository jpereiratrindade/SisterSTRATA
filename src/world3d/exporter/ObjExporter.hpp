#pragma once
#include "world3d/rendering/Vertex.hpp"
#include <vector>
#include <string>
#include <vulkan/vulkan.hpp>

namespace World3D {
namespace Exporter {

class ObjExporter {
public:
    static bool save(const std::string& path, const std::vector<Rendering::Vertex>& vertices, vk::PrimitiveTopology topology);
};

} // namespace Exporter
} // namespace World3D
