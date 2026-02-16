#pragma once

#include <vulkan/vulkan.h>

#include "RHI/Image/Image.hpp"

namespace BarrierRecipes {
    VkImageMemoryBarrier RawDefault(VkImage);
    VkImageMemoryBarrier Default(const Image& image);
    VkImageMemoryBarrier TransferDest(const Image& image);
    VkImageMemoryBarrier ShaderRead(const Image& image);
}
