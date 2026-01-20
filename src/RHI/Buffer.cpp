#include "Buffer.hpp"

#include <stdexcept>

void Buffer::init(VmaAllocator allocator, VkDeviceSize size, BufferType type) {
    init(allocator, size, type, 0);
}

void Buffer::init(VmaAllocator allocator, VkDeviceSize size, BufferType type, VkBufferUsageFlags flags) {
    this->size = size;
    this->type = type;
    this->allocator = allocator;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.flags = flags;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    switch (type) {
        case BufferType::GPU_STATIC:
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
            allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case BufferType::CPU_TO_GPU:
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferType::STAGING_UPLOAD:
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferType::READBACK:
            bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
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

void Buffer::immediateCopy(VulkanContext &ctx, Buffer &src, const size_t size) {
    VkCommandBuffer cmd = ctx.singleTimeCmdBegin(ctx.transferQueueCtx);
    copy(cmd, src, size);
    ctx.singleTimeCmdSubmit(ctx.transferQueueCtx, cmd);
}

void Buffer::immediateUpload(VulkanContext &ctx, const void* data, const size_t size) {
    VkCommandBuffer cmd = ctx.singleTimeCmdBegin(ctx.transferQueueCtx);
    upload(cmd, data, size);
    ctx.singleTimeCmdSubmit(ctx.transferQueueCtx, cmd);
}

void Buffer::copy(VkCommandBuffer &cmd, Buffer &src, const size_t size) {
    vmaCopyMemoryToAllocation(allocator, src.map(), allocation, 0, size);
}

void Buffer::upload(VkCommandBuffer &cmd, const void* data, const size_t size) {
    if (!(type == BufferType::GPU_STATIC || type == BufferType::READBACK)) {
        throw std::runtime_error("tried to upload data to a cpu-visible buffer using a command buffer for staging");
    }
    Buffer stagingBuffer;
    stagingBuffer.init(allocator, size, BufferType::STAGING_UPLOAD, 0);
    stagingBuffer.upload(data, size);
    copy(cmd, stagingBuffer, size);
    return;
}

void Buffer::upload(const void* data, const size_t size) {
    if (type == BufferType::GPU_STATIC || type == BufferType::READBACK) {
        throw std::runtime_error("tried to upload data to a gpu-only buffer without a command buffer");
    }
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