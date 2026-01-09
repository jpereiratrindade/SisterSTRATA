#include "world3d/rendering/VulkanRenderer.hpp"
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace World3D::Rendering {

VulkanRenderer::VulkanRenderer(std::shared_ptr<VulkanContext> context, uint32_t width, uint32_t height)
    : context_(context) {
    swapchain_ = std::make_unique<Swapchain>(*context_, width, height);
    createRenderPass();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
    createUniformBuffers();
    createDescriptorPool();

    pointPipeline_ = std::make_unique<Pipeline>(*context_, renderPass_, vk::PrimitiveTopology::ePointList);
    linePipeline_ = std::make_unique<Pipeline>(*context_, renderPass_, vk::PrimitiveTopology::eLineList);
    trianglePipeline_ = std::make_unique<Pipeline>(*context_, renderPass_, vk::PrimitiveTopology::eTriangleList);

    createDescriptorSets(); 
}

VulkanRenderer::~VulkanRenderer() {
    auto device = context_->getDevice();
    device.waitIdle();

    for (auto framebuffer : framebuffers_) {
        device.destroyFramebuffer(framebuffer);
    }

    device.destroyRenderPass(renderPass_);

    // Destroy pipelines (clears descriptor layout and shaders)
    pointPipeline_.reset();
    linePipeline_.reset();
    trianglePipeline_.reset();

    // Destroy sync and pools
    device.destroyDescriptorPool(descriptorPool_);
    device.destroyCommandPool(commandPool_);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroySemaphore(renderFinishedSemaphores_[i]);
        device.destroySemaphore(imageAvailableSemaphores_[i]);
        device.destroyFence(inFlightFences_[i]);
    }
}

void VulkanRenderer::createRenderPass() {
    vk::AttachmentDescription colorAttachment(
        {}, 
        swapchain_->getImageFormat(),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass(
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentRef
    );

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    vk::RenderPassCreateInfo renderPassInfo({}, 1, &colorAttachment, 1, &subpass, 1, &dependency);

    renderPass_ = context_->getDevice().createRenderPass(renderPassInfo);
}

void VulkanRenderer::createFramebuffers() {
    auto& imageViews = swapchain_->getImageViews();
    framebuffers_.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        vk::ImageView attachments[] = {
            imageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo(
            {},
            renderPass_,
            1, attachments,
            swapchain_->getExtent().width,
            swapchain_->getExtent().height,
            1
        );

        framebuffers_[i] = context_->getDevice().createFramebuffer(framebufferInfo);
    }
}

void VulkanRenderer::createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        context_->getGraphicsQueueFamilyIndex()
    );

    commandPool_ = context_->getDevice().createCommandPool(poolInfo);
}

void VulkanRenderer::createCommandBuffers() {
    vk::CommandBufferAllocateInfo allocInfo(
        commandPool_,
        vk::CommandBufferLevel::ePrimary,
        MAX_FRAMES_IN_FLIGHT
    );

    commandBuffers_ = context_->getDevice().allocateCommandBuffers(allocInfo);
}

void VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight_.assign(swapchain_->getImageCount(), nullptr);

    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores_[i] = context_->getDevice().createSemaphore(semaphoreInfo);
        renderFinishedSemaphores_[i] = context_->getDevice().createSemaphore(semaphoreInfo);
        inFlightFences_[i] = context_->getDevice().createFence(fenceInfo);
    }
}

void VulkanRenderer::beginFrame(const Camera& camera) {
    auto device = context_->getDevice();
    
    // Wait for the frame to be available
    (void)device.waitForFences(1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    try {
        auto result = device.acquireNextImageKHR(swapchain_->getHandle(), UINT64_MAX, imageAvailableSemaphores_[currentFrame_], nullptr);
        imageIndex_ = result.value;
    } catch (const vk::OutOfDateKHRError&) {
        throw std::runtime_error("Swapchain out of date!");
    }

    // Check if a previous frame is using this image (there is its fence to wait on)
    if (imagesInFlight_[imageIndex_]) {
        (void)device.waitForFences(1, &imagesInFlight_[imageIndex_], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    imagesInFlight_[imageIndex_] = inFlightFences_[currentFrame_];
    
    updateUniformBuffer(currentFrame_, camera);

    (void)device.resetFences(1, &inFlightFences_[currentFrame_]);

    auto cmd = commandBuffers_[currentFrame_];
    cmd.reset();

    vk::CommandBufferBeginInfo beginInfo;
    cmd.begin(beginInfo);

    vk::ClearValue clearColor(std::array<float, 4>{0.05f, 0.05f, 0.08f, 1.0f});
    vk::RenderPassBeginInfo renderPassInfo(
        renderPass_,
        framebuffers_[imageIndex_],
        vk::Rect2D({0, 0}, swapchain_->getExtent()),
        1, &clearColor
    );

    cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport(0.0f, 0.0f, (float)swapchain_->getExtent().width, (float)swapchain_->getExtent().height, 0.0f, 1.0f);
    cmd.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({0, 0}, swapchain_->getExtent());
    cmd.setScissor(0, 1, &scissor);
}

void VulkanRenderer::render(const Scene& scene) {
    auto cmd = commandBuffers_[currentFrame_];

    for (const auto& obj : scene.getObjects()) {
        if (!obj.vertexBuffer) continue;

        Pipeline* pipeline = nullptr;
        if (obj.topology == vk::PrimitiveTopology::ePointList) pipeline = pointPipeline_.get();
        if (obj.topology == vk::PrimitiveTopology::eLineList) pipeline = linePipeline_.get();
        if (obj.topology == vk::PrimitiveTopology::eTriangleList) pipeline = trianglePipeline_.get();

        if (pipeline) {
            pipeline->bind(cmd);
            
            // Bind Camera UBO
            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, 
                pipeline->getPipelineLayout(), 
                0, 
                1, &descriptorSets_[currentFrame_], 
                0, nullptr
            );

            vk::Buffer vertexBuffers[] = {obj.vertexBuffer->getHandle()};
            vk::DeviceSize offsets[] = {0};
            cmd.bindVertexBuffers(0, 1, vertexBuffers, offsets);
            
            // TODO: Push Constants for Model Matrix?
            // For now, identity model matrix (camera moves around world)
            
            cmd.draw(obj.vertexCount, 1, 0, 0);
        }
    }
}

void VulkanRenderer::endFrame() {
    auto cmd = commandBuffers_[currentFrame_];
    cmd.endRenderPass();
    if (!pendingScreenshotPath_.empty()) {
        recordScreenshotCopy(cmd);
    }
    cmd.end();

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo(
        1, &imageAvailableSemaphores_[currentFrame_], waitStages,
        1, &cmd,
        1, &renderFinishedSemaphores_[currentFrame_]
    );

    (void)context_->getGraphicsQueue().submit(1, &submitInfo, inFlightFences_[currentFrame_]);

    vk::SwapchainKHR swapchainHandle = swapchain_->getHandle();
    vk::PresentInfoKHR presentInfo(
        1, &renderFinishedSemaphores_[currentFrame_],
        1, &swapchainHandle,
        &imageIndex_
    );

    try {
        (void)context_->getGraphicsQueue().presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError&) {
        // Resize handled elsewhere
    }

    if (screenshotPending_) {
        auto device = context_->getDevice();
        (void)device.waitForFences(1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
        writeScreenshotToDisk();
    }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocInfo(
        commandPool_,
        vk::CommandBufferLevel::ePrimary,
        1
    );

    auto cmdBuffer = context_->getDevice().allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion(0, 0, size);
    cmdBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    cmdBuffer.end();

    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &cmdBuffer, 0, nullptr);
    if (context_->getGraphicsQueue().submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit copy buffer command!");
    }
    context_->getGraphicsQueue().waitIdle();

    context_->getDevice().freeCommandBuffers(commandPool_, 1, &cmdBuffer);
}

bool VulkanRenderer::requestScreenshot(const std::string& path) {
    if (path.empty()) return false;
    pendingScreenshotPath_ = path;
    return true;
}

void VulkanRenderer::recordScreenshotCopy(vk::CommandBuffer cmd) {
    auto extent = swapchain_->getExtent();
    screenshotWidth_ = extent.width;
    screenshotHeight_ = extent.height;
    screenshotFormat_ = swapchain_->getImageFormat();

    const vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(screenshotWidth_) * screenshotHeight_ * 4;
    screenshotBuffer_ = std::make_unique<Buffer>(
        *context_,
        imageSize,
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    auto image = swapchain_->getImages()[imageIndex_];
    vk::ImageSubresourceRange range(
        vk::ImageAspectFlagBits::eColor,
        0,
        1,
        0,
        1
    );

    vk::ImageMemoryBarrier toTransfer(
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::ePresentSrcKHR,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image,
        range
    );
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        nullptr,
        nullptr,
        toTransfer
    );

    vk::BufferImageCopy copyRegion(
        0,
        0,
        0,
        vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
        vk::Offset3D{0, 0, 0},
        vk::Extent3D{screenshotWidth_, screenshotHeight_, 1}
    );
    cmd.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, screenshotBuffer_->getHandle(), 1, &copyRegion);

    vk::ImageMemoryBarrier toPresent(
        vk::AccessFlagBits::eTransferRead,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image,
        range
    );
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        nullptr,
        nullptr,
        toPresent
    );

    screenshotPending_ = true;
}

void VulkanRenderer::writeScreenshotToDisk() {
    if (!screenshotBuffer_) {
        screenshotPending_ = false;
        pendingScreenshotPath_.clear();
        return;
    }

    void* data = nullptr;
    screenshotBuffer_->map(&data);
    const size_t imageSize = static_cast<size_t>(screenshotWidth_) * screenshotHeight_ * 4;
    const unsigned char* src = static_cast<unsigned char*>(data);

    std::vector<unsigned char> rgba(imageSize);
    for (uint32_t y = 0; y < screenshotHeight_; ++y) {
        size_t srcRow = static_cast<size_t>(y) * screenshotWidth_ * 4;
        size_t dstRow = srcRow;
        for (uint32_t x = 0; x < screenshotWidth_; ++x) {
            size_t i = srcRow + x * 4;
            size_t o = dstRow + x * 4;
            unsigned char b = src[i + 0];
            unsigned char g = src[i + 1];
            unsigned char r = src[i + 2];
            unsigned char a = src[i + 3];
            if (screenshotFormat_ == vk::Format::eB8G8R8A8Srgb || screenshotFormat_ == vk::Format::eB8G8R8A8Unorm) {
                rgba[o + 0] = r;
                rgba[o + 1] = g;
                rgba[o + 2] = b;
                rgba[o + 3] = a;
            } else {
                rgba[o + 0] = b;
                rgba[o + 1] = g;
                rgba[o + 2] = r;
                rgba[o + 3] = a;
            }
        }
    }
    screenshotBuffer_->unmap();

    WritePng(pendingScreenshotPath_, rgba, static_cast<int>(screenshotWidth_), static_cast<int>(screenshotHeight_));

    screenshotBuffer_.reset();
    screenshotPending_ = false;
    pendingScreenshotPath_.clear();
}

void VulkanRenderer::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped_.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers_[i] = std::make_unique<Buffer>(
            *context_,
            bufferSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        uniformBuffers_[i]->map(&uniformBuffersMapped_[i]);
    }
}

void VulkanRenderer::createDescriptorPool() {
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    vk::DescriptorPoolCreateInfo poolInfo(
        {},
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        1, &poolSize
    );

    descriptorPool_ = context_->getDevice().createDescriptorPool(poolInfo);
}

void VulkanRenderer::createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pointPipeline_->getDescriptorSetLayout());
    vk::DescriptorSetAllocateInfo allocInfo(
        descriptorPool_,
        static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        layouts.data()
    );

    descriptorSets_ = context_->getDevice().allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo(
            uniformBuffers_[i]->getHandle(),
            0,
            sizeof(UniformBufferObject)
        );

        vk::WriteDescriptorSet descriptorWrite(
            descriptorSets_[i],
            0, 0, 1, vk::DescriptorType::eUniformBuffer,
            nullptr, &bufferInfo, nullptr
        );

        context_->getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage, const Camera& camera) {
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = camera.getViewMatrix();
    ubo.proj = camera.getProjectionMatrix();
    
    // Lighting
    ubo.lightDir = glm::vec4(lightDir_, 0.0f);
    ubo.lightColor = glm::vec4(lightColor_, 1.0f);
    ubo.ambientStrength = ambientStrength_;
    ubo.pointSize = pointSize_;

    memcpy(uniformBuffersMapped_[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderer::setVSync(bool enabled) {
    if (vsyncEnabled_ != enabled) {
        vsyncEnabled_ = enabled;
        recreateSwapchain();
    }
}

void VulkanRenderer::recreateSwapchain() {
    auto device = context_->getDevice();
    device.waitIdle();

    int w = 0, h = 0;
    SDL_Window* window = context_->getWindow();
    SDL_Vulkan_GetDrawableSize(window, &w, &h);

    while (w == 0 || h == 0) {
        SDL_Vulkan_GetDrawableSize(window, &w, &h);
        SDL_WaitEvent(nullptr);
    }

    swapchain_->recreate(w, h, vsyncEnabled_);

    for (auto framebuffer : framebuffers_) {
        device.destroyFramebuffer(framebuffer);
    }
    createFramebuffers();

    imagesInFlight_.assign(swapchain_->getImageCount(), nullptr);
}

} // namespace World3D::Rendering
