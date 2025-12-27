#include "CsvExporter.hpp"
#include <fstream>
#include <iomanip>

namespace World3D {
namespace Exporter {

bool CsvExporter::save(const std::string& path, const std::vector<Rendering::Vertex>& vertices) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    // Header
    file << "x,y,z,r,g,b\n";

    file << std::fixed << std::setprecision(6);
    for (const auto& v : vertices) {
        file << v.pos.x << "," << v.pos.y << "," << v.pos.z << ","
             << v.color.r << "," << v.color.g << "," << v.color.b << "\n";
    }

    file.close();
    return true;
}

} // namespace Exporter
} // namespace World3D
