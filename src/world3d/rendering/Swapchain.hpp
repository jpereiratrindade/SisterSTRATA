#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "world3d/rendering/VulkanContext.hpp"

namespace World3D::Rendering {

class Swapchain {
public:
    Swapchain(VulkanContext& context, uint32_t width, uint32_t height);
    ~Swapchain();

    void recreate(uint32_t width, uint32_t height, bool vsync);
    
    [[nodiscard]] vk::SwapchainKHR getHandle() const { return swapchain_; }
    [[nodiscard]] vk::Format getImageFormat() const { return imageFormat_; }
    [[nodiscard]] vk::Extent2D getExtent() const { return extent_; }
    [[nodiscard]] const std::vector<vk::ImageView>& getImageViews() const { return imageViews_; }
    [[nodiscard]] const std::vector<vk::Image>& getImages() const { return images_; }
    [[nodiscard]] uint32_t getImageCount() const { return static_cast<uint32_t>(images_.size()); }

private:
    void createSwapchain(uint32_t width, uint32_t height, bool vsync);
    void createImageViews();
    void cleanup();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, bool vsync);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

    VulkanContext& context_;
    vk::SwapchainKHR swapchain_;
    std::vector<vk::Image> images_;
    vk::Format imageFormat_;
    vk::Extent2D extent_;
    std::vector<vk::ImageView> imageViews_;
};

} // namespace World3D::Rendering
