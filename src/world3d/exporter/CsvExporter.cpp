#include "CsvExporter.hpp"
#include <fstream>
#include <iomanip>

namespace World3D {
namespace Exporter {

bool CsvExporter::save(const std::string& path, const std::vector<Rendering::Vertex>& vertices) {
    if (vertices.empty()) return false;

    // 1. Collect & Deduplicate (Grid Recovery)
    // Use a map with integer keys (mm precision) to filter duplicates
    // Key: (x_mm << 32) | y_mm. Warning: overflows if x,y > 2000km. 
    // Use string or nested map for safety? Or just vector + sort + unique.
    // Vector sort is faster.
    
    struct PointEntry {
        glm::vec3 pos;
        glm::vec3 color;
        
        bool operator<(const PointEntry& other) const {
            // Sort by Y (primary) then X (secondary) for Row-Major Order
            // Epsilon check?
            if (std::abs(pos.y - other.pos.y) > 0.001f) return pos.y < other.pos.y;
            return pos.x < other.pos.x;
        }
    };

    std::vector<PointEntry> uniquePoints;
    uniquePoints.reserve(vertices.size());

    for (const auto& v : vertices) {
        uniquePoints.push_back({v.pos, v.color});
    }

    // Sort to bring duplicates together and order them
    std::sort(uniquePoints.begin(), uniquePoints.end());

    // Unique filter
    auto last = std::unique(uniquePoints.begin(), uniquePoints.end(), [](const PointEntry& a, const PointEntry& b) {
        return std::abs(a.pos.x - b.pos.x) < 0.001f && std::abs(a.pos.y - b.pos.y) < 0.001f;
    });
    uniquePoints.erase(last, uniquePoints.end());

    // Warning if not a square grid?
    // Not strictly necessary for CSV, but good for Drainage.
    // We just write the clean data.

    std::ofstream file(path);
    if (!file.is_open()) return false;

    // Header
    file << "x,y,z,r,g,b\n";

    file << std::fixed << std::setprecision(6);
    for (const auto& p : uniquePoints) {
        file << p.pos.x << "," << p.pos.y << "," << p.pos.z << ","
             << p.color.r << "," << p.color.g << "," << p.color.b << "\n";
    }

    file.close();
    std::cout << "[CsvExporter] Saved " << uniquePoints.size() << " unique points (Grid Recovered)." << std::endl;
    return true;
}

} // namespace Exporter
} // namespace World3D
