#pragma once

#include "core/value_objects/Vector3.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <glm/glm.hpp>

namespace Infrastructure::IO {

struct PointCloudData {
    std::vector<Core::ValueObjects::Vector3> points;
    std::vector<glm::vec3> colors;
};

/**
 * @brief Line-list data built from explicit polylines.
 */
struct PolylineData {
    std::vector<Core::ValueObjects::Vector3> points; // line list order
    std::vector<glm::vec3> colors;
};

class CsvLoader {
public:
    /**
     * @brief Load explicit polyline CSVs using line_id + seq columns.
     * Expected header: line_id, seq, x, y, z, r, g, b (colors optional).
     * @return PolylineData ready for LineList rendering (paired vertices).
     */
    static PolylineData loadPolylines(const std::string& path) {
        PolylineData data;
        std::ifstream file(path);
        if (!file.is_open()) {
            return data;
        }

        auto trim = [](std::string s) {
            auto notSpace = [](unsigned char c) { return !std::isspace(c); };
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
            s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
            return s;
        };
        auto split = [](const std::string& line) {
            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                parts.push_back(token);
            }
            return parts;
        };
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return s;
        };

        std::string header;
        while (std::getline(file, header)) {
            if (header.empty() || header[0] == '#') continue;
            header = trim(header);
            if (!header.empty()) break;
        }
        if (header.empty()) return data;

        auto headerParts = split(header);
        std::unordered_map<std::string, int> idx;
        for (size_t i = 0; i < headerParts.size(); ++i) {
            std::string key = toLower(trim(headerParts[i]));
            idx[key] = static_cast<int>(i);
        }

        auto getIdx = [&](const std::vector<std::string>& names) -> int {
            for (const auto& name : names) {
                auto it = idx.find(name);
                if (it != idx.end()) return it->second;
            }
            return -1;
        };

        int idxLine = getIdx({"line_id", "polyline_id", "boundary_id"});
        int idxSeq = getIdx({"seq", "order", "idx"});
        int idxX = getIdx({"x"});
        int idxY = getIdx({"y"});
        int idxZ = getIdx({"z"});
        int idxR = getIdx({"r"});
        int idxG = getIdx({"g"});
        int idxB = getIdx({"b"});

        if (idxLine < 0 || idxSeq < 0 || idxX < 0 || idxY < 0) {
            return data;
        }

        struct Row {
            int lineId = 0;
            int seq = 0;
            Core::ValueObjects::Vector3 point;
            glm::vec3 color;
            bool hasColor = false;
        };

        std::vector<Row> rows;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto parts = split(line);
            if (static_cast<int>(parts.size()) <= std::max({idxLine, idxSeq, idxX, idxY})) continue;

            Row row;
            try {
                row.lineId = std::stoi(parts[idxLine]);
                row.seq = std::stoi(parts[idxSeq]);
                row.point.x = std::stod(parts[idxX]);
                row.point.y = std::stod(parts[idxY]);
                row.point.z = (idxZ >= 0 && idxZ < static_cast<int>(parts.size())) ? std::stod(parts[idxZ]) : 0.0;
                if (idxR >= 0 && idxG >= 0 && idxB >= 0 &&
                    idxR < static_cast<int>(parts.size()) &&
                    idxG < static_cast<int>(parts.size()) &&
                    idxB < static_cast<int>(parts.size())) {
                    float r = static_cast<float>(std::stod(parts[idxR]));
                    float g = static_cast<float>(std::stod(parts[idxG]));
                    float b = static_cast<float>(std::stod(parts[idxB]));
                    if (r > 1.0f || g > 1.0f || b > 1.0f) {
                        r /= 255.0f;
                        g /= 255.0f;
                        b /= 255.0f;
                    }
                    row.color = glm::vec3(r, g, b);
                    row.hasColor = true;
                }
            } catch (...) {
                continue;
            }
            rows.push_back(row);
        }

        if (rows.empty()) return data;

        auto hashColor = [](int id) {
            uint32_t h = static_cast<uint32_t>(id) * 2654435761u;
            h ^= (h >> 13);
            h *= 0x9E3779B1u;
            float r = static_cast<float>((h >> 16) & 0xFF) / 255.0f;
            float g = static_cast<float>((h >> 8) & 0xFF) / 255.0f;
            float b = static_cast<float>(h & 0xFF) / 255.0f;
            if (r == 0.0f && g == 0.0f && b == 0.0f) b = 1.0f;
            return glm::vec3(r, g, b);
        };

        std::unordered_map<int, std::vector<Row>> grouped;
        for (const auto& row : rows) {
            grouped[row.lineId].push_back(row);
        }

        for (auto& kv : grouped) {
            auto& poly = kv.second;
            std::sort(poly.begin(), poly.end(), [](const Row& a, const Row& b) {
                return a.seq < b.seq;
            });

            for (size_t i = 0; i + 1 < poly.size(); ++i) {
                const auto& a = poly[i];
                const auto& b = poly[i + 1];
                glm::vec3 col = a.hasColor ? a.color : hashColor(a.lineId);
                data.points.push_back(a.point);
                data.points.push_back(b.point);
                data.colors.push_back(col);
                data.colors.push_back(col);
            }
        }

        std::cout << "[CsvLoader] Loaded " << data.points.size() << " polyline vertices from " << path << std::endl;
        return data;
    }

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
        while (std::getline(file, line) && (line.empty() || line[0] == '#')) {
            if (line.empty()) continue;
            std::string lowLine = line;
            std::transform(lowLine.begin(), lowLine.end(), lowLine.begin(), ::tolower);

            if (lowLine.rfind("# origin:", 0) == 0) {
                isGrid = true;
                std::stringstream meta(line.substr(9));
                char comma;
                if (meta >> originX >> comma >> originY) {
                    std::cout << "[CsvLoader] Origin: " << originX << ", " << originY << std::endl;
                }
            } else if (lowLine.rfind("# cell_size:", 0) == 0) {
                isGrid = true;
                std::stringstream meta(line.substr(12));
                if (meta >> cellSize) {
                    std::cout << "[CsvLoader] Cell Size: " << cellSize << std::endl;
                }
            }
        }
        
        if (isGrid) {
            std::cout << "[CsvLoader] Data identified as Grid CSV." << std::endl;
        } else {
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
                        if (std::abs(val) > 0.0001) {
                             double px = originX + x * cellSize;
                             double py = originY + y * cellSize;
                             
                             // If it's a grid CSV from SisterSTRATA, the value is likely an ID.
                             // Plotting ID as Z makes them "float" at Z=1, 2, 3...
                             // For better context, we keep it as Z, but warn the user in docs.
                             double pz = val; 
                             
                             data.points.push_back({px, py, pz});
                             
                             // Better Hash Color for Label
                             int label = static_cast<int>(val);
                             uint32_t h = static_cast<uint32_t>(label) * 0x9E3779B1u;
                             float r = ((h >> 16) & 0xFF) / 255.0f;
                             float g = ((h >> 8) & 0xFF) / 255.0f;
                             float b = (h & 0xFF) / 255.0f;
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

} // namespace Infrastructure::IO
