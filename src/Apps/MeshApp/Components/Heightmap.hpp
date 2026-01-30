#pragma once

#include <stdexcept>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "TerrainConfig.hpp"

namespace Heightmap {
    constexpr VkFormat IMAGE_FORMAT = VK_FORMAT_R16_UNORM;
    constexpr VkDeviceSize HEIGHTMAP_SIZE = TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION * sizeof(uint16_t) * TerrainConfig::INSTANCE_COUNT;

    struct HeightmapImage{
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkSampler sampler;
    };

    inline HeightmapImage create(VkDevice device, VmaAllocator allocator) {
        HeightmapImage heightmap{};

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.format = IMAGE_FORMAT;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.extent.width = TerrainConfig::RESOLUTION;
        imageCreateInfo.extent.height = TerrainConfig::RESOLUTION;
        imageCreateInfo.arrayLayers = TerrainConfig::INSTANCE_COUNT;

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

        vmaCreateImage(allocator, &imageCreateInfo, &allocCreateInfo, &heightmap.image, &heightmap.allocation, nullptr);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = heightmap.image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewCreateInfo.format = IMAGE_FORMAT;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = TerrainConfig::INSTANCE_COUNT;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &heightmap.imageView) != VK_SUCCESS) {
            throw std::runtime_error("heightmap image view creation failed");
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &heightmap.sampler) != VK_SUCCESS) {
            throw std::runtime_error("heightmap sampler creation failed");
        }

        return heightmap;
    }

    inline void destroy(HeightmapImage heightmapImage, VkDevice device, VmaAllocator allocator) {
        if (heightmapImage.image) { vmaDestroyImage(allocator, heightmapImage.image, heightmapImage.allocation); }
        if (heightmapImage.imageView) { vkDestroyImageView(device, heightmapImage.imageView, nullptr); }
        if (heightmapImage.sampler) { vkDestroySampler(device, heightmapImage.sampler, nullptr); }
    }
}
