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
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue; // Skip comments/empty

            // Replace commas with spaces for easy parsing or use getline with delimiter
            for (char& c : line) {
                if (c == ',') c = ' ';
            }

            std::stringstream ss(line);
            double x, y, z;
            float r = 0.5f, g = 0.5f, b = 0.5f;

            if (ss >> x >> y >> z) {
                // Try reading colors if available
                float tr, tg, tb;
                if (ss >> tr >> tg >> tb) {
                    // Check if 0-255 or 0-1
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
        
        std::cout << "[CsvLoader] Loaded " << data.points.size() << " points from " << path << std::endl;
        return data;
    }
};

} // namespace World3D::Loader
