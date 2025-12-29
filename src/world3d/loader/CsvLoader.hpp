#pragma once

#include "core/value_objects/Vector3.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

namespace World3D::Loader {

struct PointCloudData {
    std::vector<Core::ValueObjects::Vector3> points;
    std::vector<glm::vec3> colors;
};

class CsvLoader {
public:
    static PointCloudData load(const std::string& path) {
        PointCloudData data;
        std::ifstream file(path);
        
        if (!file.is_open()) {
            std::cerr << "[CsvLoader] Failed to open file: " << path << std::endl;
            return data;
        }

        std::string line;
        double originX = 0.0;
        double originY = 0.0;
        bool isGrid = false;
        double cellSize = 1.0; 

        // Check header for metadata
        std::getline(file, line);
        if (line.rfind("# Origin:", 0) == 0) {
            isGrid = true;
            std::stringstream meta(line.substr(9));
            char comma;
            meta >> originX >> comma >> originY;
            std::cout << "[CsvLoader] Detected Grid CSV. Origin: " << originX << ", " << originY << std::endl;
        } else {
            // Reset to beginning if not a grid header (or handle as point cloud)
            file.clear();
            file.seekg(0);
        }

        if (isGrid) {
            int y = 0;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                
                std::stringstream ss(line);
                std::string token;
                int x = 0;
                while (std::getline(ss, token, ',')) {
                    if (token.empty()) continue;
                    try {
                        double val = std::stod(token);
                        // Skip NoData/Zero if desired, or visualize everything.
                        // Let's visualize non-zero patches.
                        if (std::abs(val) > 0.001) {
                             double px = originX + x * cellSize;
                             double py = originY + y * cellSize;
                             double pz = val; // Use value as height for visibility? Or 0?
                             // Patches are usually labels (1, 2, 3). Z=1 is minimal.
                             // Z=0 might be buried.
                             // Let's use Z=val but maybe scaled?
                             // Just use raw value.
                             
                             data.points.push_back({px, py, pz});
                             
                             // Simple Hash Color for Label
                             int label = static_cast<int>(val);
                             float r = (label * 123 % 255) / 255.0f;
                             float g = (label * 456 % 255) / 255.0f;
                             float b = (label * 789 % 255) / 255.0f;
                             data.colors.push_back({r, g, b});
                        }
                    } catch(...) {}
                    x++;
                }
                y++;
            }
        } else {
            // Standard Point Cloud Loader
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue; 
                for (char& c : line) { if (c == ',') c = ' '; } // Comma to space

                std::stringstream ss(line);
                double x, y, z;
                float r = 0.5f, g = 0.5f, b = 0.5f;

                if (ss >> x >> y >> z) {
                    float tr, tg, tb;
                    if (ss >> tr >> tg >> tb) {
                        if (tr > 1.0f || tg > 1.0f || tb > 1.0f) {
                             r = tr / 255.0f; g = tg / 255.0f; b = tb / 255.0f;
                        } else {
                             r = tr; g = tg; b = tb;
                        }
                    }
                    data.points.push_back({x, y, z});
                    data.colors.push_back({r, g, b});
                }
            }
        }
        
        std::cout << "[CsvLoader] Loaded " << data.points.size() << " points from " << path << std::endl;
        return data;
    }
};

} // namespace World3D::Loader
