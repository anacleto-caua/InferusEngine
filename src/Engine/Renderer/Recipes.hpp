#pragma once

#include <vulkan/vulkan.h>

#define RECIPE_TYPE static inline

namespace Recipes {
    namespace ImageViewCreateInfo {
        RECIPE_TYPE VkImageViewCreateInfo Default(VkImage Image, VkFormat Format) {
            return {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = Image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = Format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };
        }

        RECIPE_TYPE VkImageViewCreateInfo Swapchain(VkImage Image, VkFormat Format) {
            return Default(Image, Format);
        }
    };
};
