#include "world3d/rendering/Pipeline.hpp"
#include "world3d/rendering/Vertex.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

// Forced rebuild for Vertex struct update
namespace World3D::Rendering {

Pipeline::Pipeline(VulkanContext& context, vk::RenderPass renderPass, vk::PrimitiveTopology topology, const Material& material) 
    : context_(context) {

    auto vertShaderCode = readFile(material.vertexShader);
    auto fragShaderCode = readFile(material.fragmentShader);

    vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
        {}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"
    );

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
        {}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"
    );

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex Input
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
        {},
        1, &bindingDescription,
        static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data()
    );

    // Descriptor Set Layout
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1, 
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
        nullptr
    );

    vk::DescriptorSetLayoutCreateInfo layoutInfo(
        {}, 1, &uboLayoutBinding
    );

    descriptorSetLayout_ = context_.getDevice().createDescriptorSetLayout(layoutInfo);

    // Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, topology, VK_FALSE
    );

    // Viewport & Scissor (Dynamic)
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, 
        vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise,
        VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    );

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling(
        {}, vk::SampleCountFlagBits::e1, VK_FALSE
    );

    // Color Blending (Respect Material)
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    
    if (material.alphaBlending) {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    } else {
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    vk::PipelineColorBlendStateCreateInfo colorBlending(
        {}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment
    );

    // Depth Stencil (Respect Material)
    vk::PipelineDepthStencilStateCreateInfo depthStencil(
        {}, 
        material.depthTest ? VK_TRUE : VK_FALSE, 
        material.depthWrite ? VK_TRUE : VK_FALSE,
        vk::CompareOp::eLess,
        VK_FALSE, VK_FALSE, {}, {}
    );

    // Dynamic State
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState({}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());

    // Pipeline Layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
        {}, 1, &descriptorSetLayout_
    );
    pipelineLayout_ = context_.getDevice().createPipelineLayout(pipelineLayoutInfo);

    // Pipeline Creation
    vk::GraphicsPipelineCreateInfo pipelineInfo(
        {}, 2, shaderStages, &vertexInputInfo, &inputAssembly, nullptr,
        &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
        &dynamicState, pipelineLayout_, renderPass, 0, nullptr, -1
    );

    auto result = context_.getDevice().createGraphicsPipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
    graphicsPipeline_ = result.value;

    context_.getDevice().destroyShaderModule(vertShaderModule);
    context_.getDevice().destroyShaderModule(fragShaderModule);
}

Pipeline::~Pipeline() {
    context_.getDevice().destroyPipeline(graphicsPipeline_);
    context_.getDevice().destroyPipelineLayout(pipelineLayout_);
    context_.getDevice().destroyDescriptorSetLayout(descriptorSetLayout_);
}

void Pipeline::bind(vk::CommandBuffer commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_);
}

std::vector<char> Pipeline::readFile(const std::string& filename) {
    const std::vector<std::string> searchPaths = {
        "",                         // As provided (relative to CWD)
        "bin/",                     // If running from build root
        "build/bin/",               // If running from project root
        "../",                      // If running from deep inside bin?
        "../bin/"
    };

    for (const auto& path : searchPaths) {
        std::string fullPath = path + filename;
        std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

        if (file.is_open()) {
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();
            
            std::cout << "[World3D] Loaded shader: " << fullPath << std::endl;
            return buffer;
        }
    }

    throw std::runtime_error("failed to open file: " + filename + " (Tried multiple paths)");
}

vk::ShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo(
        {}, code.size(), reinterpret_cast<const uint32_t*>(code.data())
    );

    return context_.getDevice().createShaderModule(createInfo);
}

} // namespace World3D::Rendering
