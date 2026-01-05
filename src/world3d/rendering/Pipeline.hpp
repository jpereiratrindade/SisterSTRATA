#pragma once

#include "world3d/rendering/VulkanContext.hpp"
#include "world3d/rendering/Material.hpp"
#include <string>
#include <vector>

namespace World3D::Rendering {

class Pipeline {
public:
    Pipeline(VulkanContext& context, vk::RenderPass renderPass, vk::PrimitiveTopology topology, const Material& material = Material());
    ~Pipeline();

    void bind(vk::CommandBuffer commandBuffer);
    vk::DescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout_; }
    vk::PipelineLayout getPipelineLayout() const { return pipelineLayout_; }

private:
    std::vector<char> readFile(const std::string& filename);
    vk::ShaderModule createShaderModule(const std::vector<char>& code);

    VulkanContext& context_;
    vk::PipelineLayout pipelineLayout_;
    vk::Pipeline graphicsPipeline_;
    vk::DescriptorSetLayout descriptorSetLayout_;
};

} // namespace World3D::Rendering
