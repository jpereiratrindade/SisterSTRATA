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
    static bool load(const std::string& path, std::vector<Rendering::Vertex>& vertices) {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;
        std::vector<glm::vec2> temp_uvs;
        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

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
            }
        }

        // Processing indices
        for (size_t i = 0; i < vertexIndices.size(); i++) {
            Rendering::Vertex vertex;
            
            // Positions (1-based in OBJ)
            vertex.pos = temp_positions[vertexIndices[i] - 1];
            
            // Color (White default)
            vertex.color = glm::vec3(0.8f, 0.8f, 0.8f);

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
        return true;
    }
};

} // namespace World3D::Loader
