#include "world3d/rendering/VulkanRenderer.hpp"
#include <stdexcept>
#include <iostream>

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

    createDescriptorSets(); 
}

VulkanRenderer::~VulkanRenderer() {
    auto device = context_->getDevice();
    device.waitIdle();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroySemaphore(renderFinishedSemaphores_[i]);
        device.destroySemaphore(imageAvailableSemaphores_[i]);
        device.destroyFence(inFlightFences_[i]);
    }

    device.destroyCommandPool(commandPool_);

    for (auto framebuffer : framebuffers_) {
        device.destroyFramebuffer(framebuffer);
    }

    device.destroyRenderPass(renderPass_);
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
    (void)device.waitForFences(1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    try {
        auto result = device.acquireNextImageKHR(swapchain_->getHandle(), UINT64_MAX, imageAvailableSemaphores_[currentFrame_], nullptr);
        imageIndex_ = result.value;
    } catch (const vk::OutOfDateKHRError&) {
        throw std::runtime_error("Swapchain out of date!");
    }
    
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
    context_->getGraphicsQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
    context_->getGraphicsQueue().waitIdle();

    context_->getDevice().freeCommandBuffers(commandPool_, 1, &cmdBuffer);
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

    memcpy(uniformBuffersMapped_[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderer::recreateSwapchain() {
    auto device = context_->getDevice();
    device.waitIdle();

    int w = 0, h = 0;
    SDL_Window* window = context_->getWindow();
    // Use Drawable Size for pixel-perfect match (Retina/HiDPI support in future)
    // But currently using window size since we stripped HighDPI flag.
    SDL_Vulkan_GetDrawableSize(window, &w, &h);

    // Handle minimization
    while (w == 0 || h == 0) {
        SDL_Vulkan_GetDrawableSize(window, &w, &h);
        SDL_WaitEvent(nullptr);
    }

    swapchain_->recreate(w, h);

    // Recreate Framebuffers dependent on Swapchain size
    for (auto framebuffer : framebuffers_) {
        device.destroyFramebuffer(framebuffer);
    }
    createFramebuffers();
    
    // Note: RenderPass and Pipelines usually don't need recreation on resize 
    // IF the Swapchain Image Format doesn't change.
    // Dynamic State (Viewport/Scissor) handles the size change in beginFrame command recording.
}

} // namespace World3D::Rendering
