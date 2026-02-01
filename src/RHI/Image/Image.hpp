#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct Image {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;

    uint32_t width;
    uint32_t height;
    uint16_t depth;
    uint8_t mipLevels;
    uint8_t arrayLayers;
    VkFormat format;
};

struct ImageId {
    uint32_t index;
};
