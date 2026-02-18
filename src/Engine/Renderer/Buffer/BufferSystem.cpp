#include "BufferSystem.hpp"

#include <cassert>
#include <stdexcept>

#include "Engine/Renderer/Buffer/Buffer.hpp"
#include "Engine/Renderer/RendererConfig.hpp"
#include "Engine/Renderer/Buffer/BufferCreateOptions.hpp"

void BufferSystem::init(VmaAllocator VmaAllocator) {
    this->Allocator = VmaAllocator;

    Data.clear();
    Data.reserve(RendererConfig::BufferSystem::DATA_RESERVE_CAPACITY);
    FreeIndices.clear();
    FreeIndices.reserve(RendererConfig::BufferSystem::FREE_INDICES_RESERVE_CAPACITY);
}

BufferSystem::~BufferSystem() {
    for (Buffer buffer : Data) {
        destroy(buffer);
    }
}

BufferId BufferSystem::add(BufferCreateDescription createDesc) {
    BufferId id{};
    Buffer buffer{};

    if (FreeIndices.size() > 0) {
        id = FreeIndices.back();
        FreeIndices.pop_back();
    } else {
        id.index = Data.size();
        Data.push_back(buffer);
    }

    BufferCreateOptions::BufferOptions options = BufferCreateOptions::GetBufferOptions(createDesc.memType, createDesc.usage);

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = static_cast<VkDeviceSize>(createDesc.size);
    bufferCreateInfo.usage = options.vkUsage;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = options.vmaUsage;
    allocCreateInfo.requiredFlags = options.requiredFlags;
    allocCreateInfo.flags = options.vmaFlags;

    if (vmaCreateBuffer(Allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.buffer, &buffer.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer");
    }

    buffer.size = createDesc.size;
    Data[id.index] = buffer;

    return id;
}

Buffer& BufferSystem::get(BufferId id) {
    assert(id.index < Data.size() && "Trying to access out of bounds buffer index");

    Buffer& buffer = Data[id.index];

    assert(buffer.buffer != VK_NULL_HANDLE && "Attempting to access a deleted or uninitialized buffer");

    return buffer;
}

void BufferSystem::del(BufferId id) {
    destroy(get(id));
}

void BufferSystem::copy(VkCommandBuffer &cmd, BufferId srcId, BufferId dstId, const size_t size) {
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, get(srcId).buffer, get(dstId).buffer, 1, &copyRegion);
}

void BufferSystem::copy(BufferId srcId, BufferId dstId, const size_t size) {
    vmaCopyMemoryToAllocation(Allocator, map(get(srcId).allocation), get(dstId).allocation, 0, size);
}

void BufferSystem::upload(BufferId dstId, void* upload_Data) {
    upload(dstId, upload_Data, get(dstId).size);
}

void BufferSystem::upload(VkCommandBuffer &cmd, BufferId stagingId, BufferId dstId, const void* upload_Data, const size_t size) {
    upload(stagingId, upload_Data, size);
    copy(cmd, stagingId, dstId, size);
    return;
}

void BufferSystem::upload(BufferId dstId, const void* upload_Data, const size_t size) {
    VmaAllocation alloc = get(dstId).allocation;
    memcpy(map(alloc), upload_Data, size);
    unmap(alloc);
}

void* BufferSystem::map(const VmaAllocation alloc) {
    void* mappedData;
    vmaMapMemory(Allocator, alloc, &mappedData);
    return mappedData;
}

void BufferSystem::unmap(const VmaAllocation alloc) {
    vmaUnmapMemory(Allocator, alloc);
}

void BufferSystem::destroy(Buffer buffer) {
    if (buffer.buffer) { vmaDestroyBuffer(Allocator, buffer.buffer, buffer.allocation); }
}
