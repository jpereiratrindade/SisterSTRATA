#include "world3d/rendering/Swapchain.hpp"
#include <algorithm>
#include <limits>

namespace World3D::Rendering {

Swapchain::Swapchain(VulkanContext& context, uint32_t width, uint32_t height)
    : context_(context) {
    createSwapchain(width, height);
    createImageViews();
}

Swapchain::~Swapchain() {
    cleanup();
}

void Swapchain::cleanup() {
    for (auto imageView : imageViews_) {
        context_.getDevice().destroyImageView(imageView);
    }
    if (swapchain_) {
        context_.getDevice().destroySwapchainKHR(swapchain_);
    }
}

void Swapchain::recreate(uint32_t width, uint32_t height) {
    cleanup();
    createSwapchain(width, height);
    createImageViews();
}

void Swapchain::createSwapchain(uint32_t width, uint32_t height) {
    auto device = context_.getPhysicalDevice();
    auto surface = context_.getSurface();

    auto capabilities = device.getSurfaceCapabilitiesKHR(surface);
    auto formats = device.getSurfaceFormatsKHR(surface);
    auto presentModes = device.getSurfacePresentModesKHR(surface);

    auto surfaceFormat = chooseSwapSurfaceFormat(formats);
    auto presentMode = chooseSwapPresentMode(presentModes);
    auto extent = chooseSwapExtent(capabilities, width, height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo(
        {},
        surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        0, nullptr,
        capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        VK_TRUE,
        nullptr
    );

    uint32_t queueFamilyIndex = context_.getGraphicsQueueFamilyIndex();
    // sharing mode exclusive means we don't need to specify queue families if graphics & present are same
    
    try {
        swapchain_ = context_.getDevice().createSwapchainKHR(createInfo);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    images_ = context_.getDevice().getSwapchainImagesKHR(swapchain_);
    imageFormat_ = surfaceFormat.format;
    extent_ = extent;
}

void Swapchain::createImageViews() {
    imageViews_.resize(images_.size());

    for (size_t i = 0; i < images_.size(); i++) {
        vk::ImageViewCreateInfo createInfo(
            {},
            images_[i],
            vk::ImageViewType::e2D,
            imageFormat_,
            vk::ComponentMapping(
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity
            ),
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor,
                0, 1, 0, 1
            )
        );

        imageViews_[i] = context_.getDevice().createImageView(createInfo);
    }
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = { width, height };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

} // namespace World3D::Rendering
