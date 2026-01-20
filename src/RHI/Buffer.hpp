#pragma once

#include "VulkanContext.hpp"
#include <cstddef>
#include <cstring>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

enum class BufferType {
    // STRICT GPU-ONLY.
    GPU_STATIC,
    // CPU WRITES -> GPU READS.
    CPU_TO_GPU,
    // CPU WRITES -> GPU COPIES -> GPU READS.
    STAGING_UPLOAD,
    // GPU WRITES -> CPU READS.
    READBACK
};

class Buffer {
public:
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceSize size = 0;

    VmaAllocator allocator = VK_NULL_HANDLE;
private:
    BufferType type;
public:
    Buffer() = default;
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void init(VmaAllocator allocator, VkDeviceSize size, BufferType type);
    void init(VmaAllocator allocator, VkDeviceSize size, BufferType type, VkBufferUsageFlags usage);
    void immediateCopy(VulkanContext &ctx, Buffer &src, const size_t size);
    void copy(VkCommandBuffer &cmd, Buffer &src, const size_t size);
    void copy(Buffer &src, const size_t size);
    void immediateUpload(VulkanContext &ctx, const void* data, const size_t size);
    void upload(VkCommandBuffer &cmd, Buffer &stagingBuffer, const void* data, const size_t size);
    void upload(const void* data, const size_t size);
private:
    void* map();
    void unmap();
};