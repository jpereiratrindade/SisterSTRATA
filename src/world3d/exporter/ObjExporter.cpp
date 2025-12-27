#include "ObjExporter.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace World3D {
namespace Exporter {

bool ObjExporter::save(const std::string& path, const std::vector<Rendering::Vertex>& vertices, vk::PrimitiveTopology topology) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "# SisterPEC Exported OBJ\n";
    file << "# Vertices: " << vertices.size() << "\n";

    // Write Vertices (v x y z r g b)
    // OBJ supports vertex colors as non-standard extension (e.g. MeshLab imports it)
    file << std::fixed << std::setprecision(6);
    for (const auto& v : vertices) {
        file << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z 
             << " " << v.color.r << " " << v.color.g << " " << v.color.b << "\n";
    }

    // Write Topology
    if (topology == vk::PrimitiveTopology::eTriangleList) {
        // Assume explicit non-indexed triangle list
        for (size_t i = 0; i < vertices.size(); i += 3) {
            // OBJ indices are 1-based
            file << "f " << (i + 1) << " " << (i + 2) << " " << (i + 3) << "\n";
        }
    } else if (topology == vk::PrimitiveTopology::ePointList) {
        // Export as points? 'p' tag
        // Let's do 1 point per line for safety
        for (size_t i = 0; i < vertices.size(); i++) {
            file << "p " << (i + 1) << "\n";
        }
    }

    file.close();
    return true;
}

} // namespace Exporter
} // namespace World3D
