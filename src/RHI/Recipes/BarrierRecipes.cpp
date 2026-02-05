#include "BarrierRecipes.hpp"

namespace BarrierRecipes {
    VkImageMemoryBarrier Default(Image image) {
        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = image.layout,
            .newLayout = image.layout,
            .image = image.image,
            .srcQueueFamilyIndex =VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = image.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = image.arrayLayers,
            }
        };
        return barrier;
    }

    VkImageMemoryBarrier TransferDest(Image image) {
        VkImageMemoryBarrier barrier = Default(image);
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        return barrier;
    }

    VkImageMemoryBarrier ShaderRead(Image image) {
        VkImageMemoryBarrier barrier = Default(image);
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        return barrier;
    }
}
