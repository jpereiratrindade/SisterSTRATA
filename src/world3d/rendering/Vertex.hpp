#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <array>

namespace World3D::Rendering {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription(
            0,                          // binding
            sizeof(Vertex),             // stride
            vk::VertexInputRate::eVertex // inputRate
        );
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions;

        // Position
        attributeDescriptions[0] = vk::VertexInputAttributeDescription(
            0,                          // location
            0,                          // binding
            vk::Format::eR32G32B32Sfloat, // format (vec3)
            offsetof(Vertex, pos)       // offset
        );

        // Color
        attributeDescriptions[1] = vk::VertexInputAttributeDescription(
            1,                          // location
            0,                          // binding
            vk::Format::eR32G32B32Sfloat, // format (vec3)
            offsetof(Vertex, color)     // offset
        );

        // Normal
        attributeDescriptions[2] = vk::VertexInputAttributeDescription(
            2,                          // location
            0,                          // binding
            vk::Format::eR32G32B32Sfloat, // format (vec3)
            offsetof(Vertex, normal)    // offset
        );

        // UV
        attributeDescriptions[3] = vk::VertexInputAttributeDescription(
            3,                          // location
            0,                          // binding
            vk::Format::eR32G32Sfloat,  // format (vec2)
            offsetof(Vertex, uv)        // offset
        );

        return attributeDescriptions;
    }
};

} // namespace World3D::Rendering
