#pragma once

#include "world3d/rendering/VulkanContext.hpp"
#include "world3d/rendering/Swapchain.hpp"
#include "world3d/rendering/Pipeline.hpp"
#include "world3d/rendering/Buffer.hpp"
#include "world3d/rendering/Vertex.hpp"
#include <memory>
#include <vector>

#include "world3d/scene/Scene.hpp"
#include "world3d/camera/Camera.hpp"
#include <glm/glm.hpp>

namespace World3D::Rendering {
// Forward declaration if needed, but we included Scene.hpp


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class VulkanRenderer {
public:
    VulkanRenderer(std::shared_ptr<VulkanContext> context, uint32_t width, uint32_t height);
    ~VulkanRenderer();

    void beginFrame(const Camera& camera); 
    void render(const Scene& scene); // New
    void endFrame();
    void recreateSwapchain(); // Handling Resize
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    [[nodiscard]] vk::RenderPass getRenderPass() const { return renderPass_; }
    [[nodiscard]] vk::CommandPool getCommandPool() const { return commandPool_; }
    [[nodiscard]] const std::vector<vk::CommandBuffer>& getCommandBuffers() const { return commandBuffers_; }
    [[nodiscard]] uint32_t getCurrentFrameIndex() const { return currentFrame_; }
    [[nodiscard]] uint32_t getImageIndex() const { return imageIndex_; }
    [[nodiscard]] Swapchain& getSwapchain() { return *swapchain_; }

private:
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createUniformBuffers(); // New
    void createDescriptorPool(); // New
    void createDescriptorSets(); // New
    void updateUniformBuffer(uint32_t currentImage, const Camera& camera); // New

    std::shared_ptr<VulkanContext> context_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<Pipeline> pointPipeline_;
    std::unique_ptr<Pipeline> linePipeline_;
    
    // Buffers removed - now in Scene objects


    vk::RenderPass renderPass_;
    std::vector<vk::Framebuffer> framebuffers_;
    
    vk::CommandPool commandPool_;
    std::vector<vk::CommandBuffer> commandBuffers_;

    // Uniform Buffers
    std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
    std::vector<void*> uniformBuffersMapped_;

    // Descriptors
    vk::DescriptorPool descriptorPool_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    // Sync objects
    std::vector<vk::Semaphore> imageAvailableSemaphores_;
    std::vector<vk::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::Fence> inFlightFences_;
    
    uint32_t currentFrame_ = 0;
    uint32_t imageIndex_ = 0;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

} // namespace World3D::Rendering
