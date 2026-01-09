#pragma once

#include "world3d/rendering/VulkanContext.hpp"
#include "world3d/rendering/Swapchain.hpp"
#include "world3d/rendering/Pipeline.hpp"
#include "world3d/rendering/Buffer.hpp"
#include "world3d/rendering/ImageWriter.hpp"
#include "world3d/rendering/Vertex.hpp"
#include <memory>
#include <vector>

#include "world3d/scene/Scene.hpp"
#include "world3d/camera/Camera.hpp"
#include <glm/glm.hpp>
#include <string>

namespace World3D::Rendering {
// Forward declaration if needed, but we included Scene.hpp


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    
    // Lighting Params
    alignas(16) glm::vec4 lightDir;   // Direction TO light? Or FROM light. Usually to source.
    alignas(16) glm::vec4 lightColor;
    alignas(4)  float ambientStrength;
    alignas(4)  float pointSize;
    alignas(8)  glm::vec2 _padding;
};

/**
 * @brief Handles the Vulkan rendering pipeline, including Swapchain management and command buffer recording.
 */
class VulkanRenderer {
public:
    /**
     * @brief Initializes the renderer, creates semaphores, and prepares the graphics pipeline.
     */
    VulkanRenderer(std::shared_ptr<VulkanContext> context, uint32_t width, uint32_t height);
    ~VulkanRenderer();

    /**
     * @brief Begins frame recording. Acquires the next image from the swapchain.
     * @param camera The active scene camera for Value updates.
     */
    void beginFrame(const Camera& camera); 
    
    /**
     * @brief Records drawing commands for the provided scene geometry.
     * @param scene The scene containing vertices/indices to draw.
     */
    void render(const Scene& scene); // New
    
    /**
     * @brief Ends frame recording and submits the command buffer to the graphics queue.
     * Presents the image to the screen.
     */
    void endFrame();
    void recreateSwapchain(); // Handling Resize
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    bool requestScreenshot(const std::string& path);

    [[nodiscard]] vk::RenderPass getRenderPass() const { return renderPass_; }
    [[nodiscard]] vk::CommandPool getCommandPool() const { return commandPool_; }
    [[nodiscard]] const std::vector<vk::CommandBuffer>& getCommandBuffers() const { return commandBuffers_; }
    [[nodiscard]] uint32_t getCurrentFrameIndex() const { return currentFrame_; }
    [[nodiscard]] uint32_t getImageIndex() const { return imageIndex_; }
    [[nodiscard]] Swapchain& getSwapchain() { return *swapchain_; }

    void updateUniformBuffer(uint32_t currentImage, const Camera& camera); // Reverted
    
    // Lighting Control
    void setLightParams(const glm::vec3& dir, const glm::vec3& color, float ambient) {
        lightDir_ = dir;
        lightColor_ = color;
        ambientStrength_ = ambient;
    }
    void setPointSize(float size) { pointSize_ = size; }

    void setVSync(bool enabled);
    bool isVSyncEnabled() const { return vsyncEnabled_; }

private:
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createUniformBuffers(); // New
    void createDescriptorPool(); // New
    void createDescriptorSets(); // New
    void recordScreenshotCopy(vk::CommandBuffer cmd);
    void writeScreenshotToDisk();


    std::shared_ptr<VulkanContext> context_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<Pipeline> pointPipeline_;
    std::unique_ptr<Pipeline> linePipeline_;
    std::unique_ptr<Pipeline> trianglePipeline_;
    
    // Lighting State
    glm::vec3 lightDir_ = glm::vec3(0.5f, 1.0f, 2.0f);
    glm::vec3 lightColor_ = glm::vec3(1.0f);
    float ambientStrength_ = 0.3f;
    float pointSize_ = 4.0f;
    
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
    bool vsyncEnabled_ = true;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    std::string pendingScreenshotPath_;
    std::unique_ptr<Buffer> screenshotBuffer_;
    uint32_t screenshotWidth_ = 0;
    uint32_t screenshotHeight_ = 0;
    vk::Format screenshotFormat_ = vk::Format::eUndefined;
    bool screenshotPending_ = false;
};

} // namespace World3D::Rendering
