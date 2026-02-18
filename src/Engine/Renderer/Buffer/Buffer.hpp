#pragma once

#include <cstddef>

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

enum class BufferMemoryType {
    // STRICT GPU-ONLY.
    GPU_STATIC,
    // CPU WRITES -> GPU READS.
    CPU_TO_GPU,
    // CPU WRITES -> GPU COPIES -> GPU READS.
    STAGING_UPLOAD,
    // GPU WRITES -> CPU READS.
    READBACK,

    _BUFFER_MEMORY_TYPE_COUNT_
};

enum class BufferUsage {
    VERTEX,
    INDEX,
    SSBO,
    UBO,
    STAGING,

    _BUFFER_USAGE_COUNT_
};

struct BufferId {
    uint32_t index;
};

struct Buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    size_t size;
    BufferMemoryType memType;
    BufferUsage usage;
};
