#include "SoilRasterizer.hpp"
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include "core/domain/soils/SiBCS.hpp"

namespace Core::Domain::SpatialPattern {

GridData SoilRasterizer::Rasterize(const std::vector<Core::ValueObjects::TerrainVertex>& vertices, 
                                   const std::vector<Core::Domain::Soils::SiBCSClassification>& classes,
                                   double cellSize) {
    GridData grid;
    if (vertices.empty()) return grid;
    if (vertices.size() != classes.size()) {
        std::cerr << "[SoilRasterizer] Error: Mismatch between vertices and classes count." << std::endl;
        return grid;
    }

    // 1. Calculate Bounding Box
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& v : vertices) {
        if (v.pos.x < minX) minX = v.pos.x;
        if (v.pos.x > maxX) maxX = v.pos.x;
        if (v.pos.y < minY) minY = v.pos.y;
        if (v.pos.y > maxY) maxY = v.pos.y;
    }

    // 2. Determine Grid Dimensions
    double widthWorld = maxX - minX;
    double heightWorld = maxY - minY;
    
    // Safety padding
    widthWorld += cellSize * 0.5;
    heightWorld += cellSize * 0.5;

    int cols = static_cast<int>(std::ceil(widthWorld / cellSize));
    int rows = static_cast<int>(std::ceil(heightWorld / cellSize));

    if (cols <= 0) cols = 1;
    if (rows <= 0) rows = 1;

    grid.width = cols;
    grid.height = rows;
    grid.cellWidth = cellSize;
    grid.cellHeight = cellSize;
    // Initialize with 0 (NoData / Background)
    size_t count = static_cast<size_t>(cols) * rows;
    grid.values.assign(count, 0.0);
    grid.elevation.assign(count, 0.0); // Init elevation

    // 3. Bin Vertices
    // Strategy: Last-wins for now. 
    // TODO: support Voting/Mode for higher accuracy if cellSize >> mesh resolution.
    for (size_t i = 0; i < vertices.size(); ++i) {
        double vx = vertices[i].pos.x - minX;
        double vy = vertices[i].pos.y - minY;
        
        int gx = static_cast<int>(vx / cellSize);
        int gy = static_cast<int>(vy / cellSize); // origin bottom-left? Assume standard cartesian.

        // Clamp to be safe
        if (gx < 0) gx = 0;
        if (gx >= cols) gx = cols - 1;
        if (gy < 0) gy = 0;
        if (gy >= rows) gy = rows - 1;

        // Map Class to ID
        // Analysis assumes ID > 0.
        // We can use Order as ID if Level 1, or generate a hash/lookup.
        // For visualizing Patches, usually "Order" or "Suborder" is interest.
        // Let's pack Order + Suborder? Or just use a customized ID mapping?
        // Simple approach: Use (int)Order * 100 + (int)Suborder? 
        // Or simplified: Just cast Order to ID for Level 1 analysis.
        // Let's assume Level 2 (Suborder) is the goal as it gives color.
        
        // Let's use a composite ID: Order(1-13) << 8 | Suborder(1-255)
        // Actually PatchAnalysis just needs unique IDs for classes.
        // Let's strictly use the Order enum for testing Level 1 first.
        
        int classId = static_cast<int>(classes[i].order); 
        // If we want suborder:
        // int classId = (static_cast<int>(classes[i].order) * 100) + static_cast<int>(classes[i].suborder);
        
        size_t idx = gy * cols + gx;
        grid.values[idx] = static_cast<double>(classId);
        grid.elevation[idx] = vertices[i].pos.z; 
    }
    
    // Store origin so consumers know where this grid sits in world space
    grid.originX = minX;
    grid.originY = minY;

    return grid;
}

void SoilRasterizer::SaveLegendCsv(const std::string& path, const std::vector<Core::Domain::Soils::SiBCSClassification>& uniqueClasses) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "id,name,r,g,b\n";
    // We need to know which IDs we used in Rasterize. 
    // Currently Rasterize uses Order.
    // This helper might need to be smarter or synchronized.
    
    // For now, let's just dump the SiBCS colors for the known classes.
    for (const auto& c : uniqueClasses) {
        int id = static_cast<int>(c.order);
        glm::vec3 color = Core::Domain::Soils::SiBCSHelper::getColor(c, 1); // Level 1 color
        file << id << "," << Core::Domain::Soils::SiBCSHelper::getName(c, 1) << "," 
             << (int)(color.r * 255) << "," << (int)(color.g * 255) << "," << (int)(color.b * 255) << "\n";
    }
}

} // namespace Core::Domain::SpatialPattern
