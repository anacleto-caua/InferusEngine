#pragma once

#include <vulkan/vulkan.h>

#include "RHI/Image/Image.hpp"

namespace BarrierRecipes {
    VkImageMemoryBarrier Default(Image image);
    VkImageMemoryBarrier TransferDest(Image image);
    VkImageMemoryBarrier ShaderRead(Image image);
}
