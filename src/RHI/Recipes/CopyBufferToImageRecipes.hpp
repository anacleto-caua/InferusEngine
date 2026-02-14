#pragma once

#include <vulkan/vulkan.h>

#include "RHI/Image/Image.hpp"

namespace CopyBufferToImageBuilder {
   VkBufferImageCopy DefaultCopy(Image& image);
}
