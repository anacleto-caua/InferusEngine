#include "Buffer.hpp"

#include <stdexcept>

Buffer::Buffer(VmaAllocator allocator, VkDeviceSize size, BufferType type) {
    this->size = size;
    this->type = type;
    this->allocator = allocator;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    switch (type) {
        case BufferType::GPU_STATIC:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
            allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case BufferType::CPU_TO_GPU:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferType::STAGING_UPLOAD:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferType::READBACK:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
    }

    if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("buffer creation failed");
    }
}

Buffer::~Buffer() {
    if (buffer) { vmaDestroyBuffer(allocator, buffer, allocation); }
}

void Buffer::upload(void* data, size_t size) {
    void* mappedData = map();
    memcpy(mappedData, data, size);
    unmap();
}

void* Buffer::map() {
    void* mappedData;
    vmaMapMemory(allocator, allocation, &mappedData);
    return mappedData;
}

void Buffer::unmap() {
    vmaUnmapMemory(allocator, allocation);
}