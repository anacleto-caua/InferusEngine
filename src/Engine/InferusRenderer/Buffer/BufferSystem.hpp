#pragma once

#include <vector>

#include "Engine/InferusRenderer/Buffer/Buffer.hpp"
#include "Engine/InferusRenderer/Buffer/BufferCreateDescription.hpp"

class BufferSystem {
public:
    VmaAllocator Allocator;
private:
    std::vector<Buffer> Data;
    std::vector<BufferId> FreeIndices;

public:
    BufferSystem() = default;
    ~BufferSystem() = default;
    BufferSystem(const BufferSystem&) = delete;
    BufferSystem& operator=(const BufferSystem&) = delete;

    void create(VmaAllocator VmaAllocator);
    void destroy();

    BufferId add(BufferCreateDescription createDesc);
    Buffer& get(BufferId id);

    void copy(BufferId srcId, BufferId dstId, const size_t size);
    void copy(VkCommandBuffer &cmd, BufferId srcId, BufferId dstId, const size_t size);

    void upload(BufferId dstId, void* upload_data);
    void upload(BufferId dstId, const void* upload_data, const size_t size);
    void upload(VkCommandBuffer &cmd, BufferId stagingId, BufferId dstId, const void* upload_data, const size_t size);

    void del(BufferId id);
private:
    void destroy(Buffer& buffer);

    void* map(const VmaAllocation alloc);
    void unmap(const VmaAllocation alloc);
};
