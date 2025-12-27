#include "world3d/rendering/Buffer.hpp"
#include <stdexcept>

namespace World3D::Rendering {

Buffer::Buffer(VulkanContext& context, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    : context_(context), size_(size) {
    
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);

    buffer_ = context_.getDevice().createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = context_.getDevice().getBufferMemoryRequirements(buffer_);

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size,
        findMemoryType(memRequirements.memoryTypeBits, properties)
    );

    memory_ = context_.getDevice().allocateMemory(allocInfo);

    context_.getDevice().bindBufferMemory(buffer_, memory_, 0);
}

Buffer::~Buffer() {
    if (buffer_) context_.getDevice().destroyBuffer(buffer_);
    if (memory_) context_.getDevice().freeMemory(memory_);
}

Buffer::Buffer(Buffer&& other) noexcept 
    : context_(other.context_), 
      buffer_(other.buffer_), 
      memory_(other.memory_), 
      size_(other.size_) 
{
    other.buffer_ = nullptr;
    other.memory_ = nullptr;
    other.size_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        // Cleanup current
        if (buffer_) context_.getDevice().destroyBuffer(buffer_);
        if (memory_) context_.getDevice().freeMemory(memory_);

        // Steal
        buffer_ = other.buffer_;
        memory_ = other.memory_;
        size_ = other.size_;
        // Reference context_ must assume same context or we can't rebind reference. 
        // Logic error design if transferring between contexts, but here it's likely same app instance.

        other.buffer_ = nullptr;
        other.memory_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

void Buffer::map(void** data) {
    *data = context_.getDevice().mapMemory(memory_, 0, size_);
}

void Buffer::unmap() {
    context_.getDevice().unmapMemory(memory_);
}

void Buffer::copyTo(const void* data, vk::DeviceSize size) {
    void* mappedData;
    map(&mappedData);
    memcpy(mappedData, data, (size_t)size);
    unmap();
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = context_.getPhysicalDevice().getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

} // namespace World3D::Rendering
