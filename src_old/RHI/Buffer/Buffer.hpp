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
    READBACK
};

enum class BufferUsage {
    VERTEX,
    INDEX,
    SSBO,
    UBO,
    STAGING
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
