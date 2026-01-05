#pragma once
#include <string>
#include <glm/glm.hpp>

namespace World3D::Rendering {

/**
 * @brief Represents a surface description (Shaders + Parameters).
 * Allows for data-driven rendering evolution.
 */
struct Material {
    std::string name = "Default";
    
    // Shader Paths (Relative to binary or registered search paths)
    std::string vertexShader = "shaders/simple.vert.spv";
    std::string fragmentShader = "shaders/simple.frag.spv";
    
    // Base parameters
    glm::vec4 baseColor = glm::vec4(1.0f);
    float roughness = 0.5f;
    float metallic = 0.0f;
    
    // Pipeline hints
    bool depthTest = true;
    bool depthWrite = true;
    bool alphaBlending = false;
};

} // namespace World3D::Rendering
