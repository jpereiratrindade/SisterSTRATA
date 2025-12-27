#pragma once

#include "world3d/rendering/VulkanContext.hpp"

namespace World3D::Rendering {

class Buffer {
public:
    Buffer(VulkanContext& context, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    ~Buffer();

    // No Copy
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Move
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    void map(void** data);
    void unmap();
    void copyTo(const void* data, vk::DeviceSize size);

    [[nodiscard]] vk::Buffer getHandle() const { return buffer_; }
    [[nodiscard]] vk::DeviceMemory getMemory() const { return memory_; }
    [[nodiscard]] vk::DeviceSize getSize() const { return size_; }

private:
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    VulkanContext& context_;
    vk::Buffer buffer_;
    vk::DeviceMemory memory_;
    vk::DeviceSize size_;
};

} // namespace World3D::Rendering
