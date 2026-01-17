#include "Buffer.hpp"

void Buffer::init(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage) {
    this->allocator = allocator;
    this->size = size;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usageFlags;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = memoryUsage;

    vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, nullptr);
}

Buffer::~Buffer() {
    if (buffer) { vmaDestroyBuffer(allocator, buffer, allocation); }
}

void Buffer::upload(void* data, size_t size) {
    void* mappedData;
    vmaMapMemory(allocator, allocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(allocator, allocation);
}