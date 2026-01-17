#pragma once

#include <cstring>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class Buffer {
public:
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceSize size = 0;

    VmaAllocator allocator = VK_NULL_HANDLE;
private:
public:
    Buffer() = default;
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void init(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
    void upload(void* data, size_t dataSize);
private:

};