#pragma once

#include "world3d/rendering/Vertex.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace World3D::Loader {

class ObjLoader {
public:
    /**
     * @brief Load an OBJ file, supporting faces and point-only geometry.
     * @param path File path.
     * @param vertices Output vertex buffer.
     * @param isPointCloud Optional flag set to true when geometry is point-only.
     * @return true if any vertices were loaded.
     */
    static bool load(const std::string& path, std::vector<Rendering::Vertex>& vertices, bool* isPointCloud = nullptr) {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_colors;
        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<unsigned int> pointIndices;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open OBJ: " << path << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                ss >> pos.x >> pos.y >> pos.z;
                temp_positions.push_back(pos);
                float r = 0.8f, g = 0.8f, b = 0.8f;
                if (ss >> r >> g >> b) {
                    if (r > 1.0f || g > 1.0f || b > 1.0f) {
                        r /= 255.0f;
                        g /= 255.0f;
                        b /= 255.0f;
                    }
                    temp_colors.push_back(glm::vec3(r, g, b));
                } else {
                    temp_colors.push_back(glm::vec3(0.8f, 0.8f, 0.8f));
                }
            } else if (prefix == "vn") {
                glm::vec3 norm;
                ss >> norm.x >> norm.y >> norm.z;
                temp_normals.push_back(norm);
            } else if (prefix == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                temp_uvs.push_back(uv);
            } else if (prefix == "f") {
                // Simplified parser: valid for triangulated faces only: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
                // Or f v1//vn1 v2//vn2 v3//vn3
                std::string v1_str, v2_str, v3_str;
                ss >> v1_str >> v2_str >> v3_str;
                
                // Helper lambda to parse indices
                auto parseIndex = [&](const std::string& str) {
                    // This is minimal. Robust OBJ parsing needs tinyobjloader.
                    // Assuming format v//vn for now as generic Blender export without UV
                    // or v/vt/vn
                    int vIdx = 0, vtIdx = 0, vnIdx = 0;
                    size_t firstSlash = str.find('/');
                    size_t secondSlash = str.find('/', firstSlash + 1);

                    if (firstSlash != std::string::npos) {
                        vIdx = std::stoi(str.substr(0, firstSlash));
                        
                        if (secondSlash != std::string::npos) {
                            // v/vt/vn or v//vn
                            if (secondSlash - firstSlash > 1) {
                                // v/vt/vn
                                // vtIdx = std::stoi(str.substr(firstSlash + 1, secondSlash - firstSlash - 1));
                            }
                            vnIdx = std::stoi(str.substr(secondSlash + 1));
                        }
                    } else {
                        vIdx = std::stoi(str);
                    }
                    
                    vertexIndices.push_back(vIdx);
                    if (vnIdx != 0) normalIndices.push_back(vnIdx);
                };

                parseIndex(v1_str);
                parseIndex(v2_str);
                parseIndex(v3_str);
            } else if (prefix == "p") {
                std::string token;
                while (ss >> token) {
                    if (token.empty()) continue;
                    size_t slash = token.find('/');
                    if (slash != std::string::npos) {
                        token = token.substr(0, slash);
                    }
                    try {
                        int vIdx = std::stoi(token);
                        if (vIdx > 0) {
                            pointIndices.push_back(static_cast<unsigned int>(vIdx));
                        }
                    } catch (...) {
                        continue;
                    }
                }
            }
        }

        if (vertexIndices.empty() && pointIndices.empty() && !temp_positions.empty()) {
            pointIndices.reserve(temp_positions.size());
            for (size_t i = 0; i < temp_positions.size(); ++i) {
                pointIndices.push_back(static_cast<unsigned int>(i + 1));
            }
        }

        const bool pointMode = vertexIndices.empty() && !pointIndices.empty();
        if (isPointCloud) {
            *isPointCloud = pointMode;
        }

        if (pointMode) {
            for (size_t i = 0; i < pointIndices.size(); ++i) {
                unsigned int idx = pointIndices[i];
                if (idx == 0 || idx > temp_positions.size()) continue;
                Rendering::Vertex vertex;
                vertex.pos = temp_positions[idx - 1];
                if (idx - 1 < temp_colors.size()) {
                    vertex.color = temp_colors[idx - 1];
                } else {
                    vertex.color = glm::vec3(0.8f, 0.8f, 0.8f);
                }
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                vertex.uv = glm::vec2(0.0f, 0.0f);
                vertices.push_back(vertex);
            }
            std::cout << "Loaded OBJ: " << path << " (" << vertices.size() << " points)" << std::endl;
            return !vertices.empty();
        }

        // Processing indices
        for (size_t i = 0; i < vertexIndices.size(); i++) {
            Rendering::Vertex vertex;
            
            // Positions (1-based in OBJ)
            vertex.pos = temp_positions[vertexIndices[i] - 1];
            
            // Color (White default)
            if (vertexIndices[i] - 1 < temp_colors.size()) {
                vertex.color = temp_colors[vertexIndices[i] - 1];
            } else {
                vertex.color = glm::vec3(0.8f, 0.8f, 0.8f);
            }

            // Normals
            if (!temp_normals.empty() && i < normalIndices.size()) {
                vertex.normal = temp_normals[normalIndices[i] - 1];
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            
            vertex.uv = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        std::cout << "Loaded OBJ: " << path << " (" << vertices.size() << " vertices)" << std::endl;
        return !vertices.empty();
    }
};

} // namespace World3D::Loader
