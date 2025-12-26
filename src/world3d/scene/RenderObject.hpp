#pragma once

#include "world3d/rendering/Buffer.hpp"
#include <vulkan/vulkan.hpp>
#include <memory>
#include <glm/glm.hpp>

namespace World3D {

struct RenderObject {
    std::shared_ptr<Rendering::Buffer> vertexBuffer;
    uint32_t vertexCount;
    vk::PrimitiveTopology topology;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    // Material/Pipeline reference could go here later
};

} // namespace World3D
