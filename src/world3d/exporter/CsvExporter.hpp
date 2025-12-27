#pragma once
#include "world3d/rendering/Vertex.hpp"
#include <vector>
#include <string>

namespace World3D {
namespace Exporter {

class CsvExporter {
public:
    static bool save(const std::string& path, const std::vector<Rendering::Vertex>& vertices);
};

} // namespace Exporter
} // namespace World3D
