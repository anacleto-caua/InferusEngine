#include "RHI/Buffer/BufferManager.hpp"

#include <cassert>

#include "RHI/RHIConfig.hpp"
#include "RHI/Buffer/Buffer.hpp"
#include "RHI/VulkanContext.hpp"
#include "RHI/Buffer/BufferCreateOptions.hpp"

void BufferManager::init(VmaAllocator allocator) {
    this->allocator = allocator;

    data.reserve(RHIConfig::BufferManagerConfig::BufferReserveCapacity);
    freeIndices.reserve(RHIConfig::BufferManagerConfig::FreeIndicesReserveCapacity);
}

BufferManager::~BufferManager() {
    for(Buffer buffer: data) {
        destroy(buffer);
    }
}

BufferId BufferManager::add(BufferCreateDescription createDesc) {
    BufferId id{};
    Buffer buffer{};

    if (freeIndices.size() > 0) {
        id = freeIndices.back();
        freeIndices.pop_back();
    } else {
        id.index = data.size();
        data.push_back(buffer);
    }

    BufferCreateOptions::BufferOptions options = BufferCreateOptions::getBufferOptions(createDesc.memType, createDesc.usage);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = createDesc.size;
    bufferCreateInfo.usage = options.vkUsage;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = options.vmaUsage;
    allocCreateInfo.requiredFlags = options.requiredFlags;
    allocCreateInfo.flags = options.vmaFlags;

    vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.buffer, &buffer.allocation, nullptr);

    return id;
}

Buffer& BufferManager::get(BufferId id) {
    assert(id.index < data.size() && "Trying to access out of bounds buffer index");

    Buffer& buffer = data[id.index];

    assert(buffer.buffer != VK_NULL_HANDLE && "Attempting to access a deleted or uninitialized buffer");

    return buffer;
}

void BufferManager::del(BufferId id) {
    destroy(get(id));
}

void BufferManager::copy(VkCommandBuffer &cmd, BufferId srcId, BufferId dstId, const size_t size) {
        // throw std::runtime_error("tried to copy data to a cpu-only buffer using a gpu method");
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, get(srcId).buffer, get(dstId).buffer, 1, &copyRegion);
}

void BufferManager::copy(BufferId srcId, BufferId dstId, const size_t size) {
        //throw std::runtime_error("tried to copy data to a gpu-only buffer with a cpu-visible dedicated method");
    vmaCopyMemoryToAllocation(allocator, map(get(srcId).allocation), get(dstId).allocation, 0, size);
}

void BufferManager::upload(BufferId dstId, void* data) {
    upload(dstId, data, get(dstId).size);
}

void BufferManager::upload(VkCommandBuffer &cmd, BufferId stagingId, BufferId dstId, const void* data, const size_t size) {
        //throw std::runtime_error("tried to upload data to a cpu-visible buffer using a command buffer for staging");
    upload(stagingId, data, size);
    copy(cmd, stagingId, dstId, size);
    return;
}

void BufferManager::upload(BufferId dstId, const void* data, const size_t size) {
        //throw std::runtime_error("tried to upload data to a gpu-only buffer without a command buffer");
    VmaAllocation alloc = get(dstId).allocation;
    memcpy(map(alloc), data, size);
    unmap(alloc);
}

void BufferManager::immediateCopy(VulkanContext &ctx, BufferId srcId, BufferId dstId, const size_t size) {
    VkCommandBuffer cmd = ctx.singleTimeCmdBegin(ctx.transferQueueCtx);
    copy(cmd, srcId, dstId, size);
    ctx.singleTimeCmdSubmit(ctx.transferQueueCtx, cmd);
}

void BufferManager::immediateUpload(VulkanContext &ctx, BufferId dstId, const void* data, const size_t size) {
    VkCommandBuffer cmd = ctx.singleTimeCmdBegin(ctx.transferQueueCtx);
    BufferCreateDescription stagingBufferDesc {
        .size = size,
        .memType = BufferMemoryType::STAGING_UPLOAD,
        .usage = BufferUsage::STAGING
    };

    BufferId stagingId = add(stagingBufferDesc);
    upload(cmd, stagingId, dstId, data, size);
    ctx.singleTimeCmdSubmit(ctx.transferQueueCtx, cmd);
}

void* BufferManager::map(const VmaAllocation alloc) {
    void* mappedData;
    vmaMapMemory(allocator, alloc, &mappedData);
    return mappedData;
}

void BufferManager::unmap(const VmaAllocation alloc) {
    vmaUnmapMemory(allocator, alloc);
}

void BufferManager::destroy(Buffer buffer) {
    if (buffer.buffer) { vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation); }
}
