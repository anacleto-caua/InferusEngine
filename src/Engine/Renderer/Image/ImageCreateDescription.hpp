#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

struct ImageCreateDescription {
    uint32_t width;
    uint32_t height;

    uint16_t depth = 1;
    uint8_t mipLevels = 1;
    uint8_t arrayLayers = 1;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
};
