#pragma once

#include <vector>

#include "RHI/VulkanContext.hpp"
#include "RHI/Buffer/Buffer.hpp"
#include "RHI/Buffer/BufferCreateDescription.hpp"

class BufferManager {
public:
    VmaAllocator allocator;
private:
    std::vector<Buffer> data;
    std::vector<BufferId> freeIndices;

public:
    BufferManager() = default;
    ~BufferManager();
    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    void init(VmaAllocator allocator);
    BufferId add(BufferCreateDescription createDesc);
    Buffer& get(BufferId id);

    void copy(BufferId srcId, BufferId dstId, const size_t size);
    void copy(VkCommandBuffer &cmd, BufferId srcId, BufferId dstId, const size_t size);

    void upload(BufferId dstId, void* data);
    void upload(BufferId dstId, const void* data, const size_t size);
    void upload(VkCommandBuffer &cmd, BufferId stagingId, BufferId dstId, const void* data, const size_t size);

    void immediateCopy(VulkanContext &ctx, BufferId srcId, BufferId dstId, const size_t size);
    void immediateUpload(VulkanContext &ctx, BufferId dstId, const void* data, const size_t size);

    void del(BufferId id);
private:
    void destroy(Buffer buffer);
    void* map(const VmaAllocation alloc);
    void unmap(const VmaAllocation alloc);
};
