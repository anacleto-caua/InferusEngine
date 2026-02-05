#pragma once

#include <vulkan/vulkan.h>

#include "RHI/Image/Image.hpp"
#include "RHI/Buffer/Buffer.hpp"

namespace CopyBufferToImageBuilder {
   VkBufferImageCopy DefaultCopy(Buffer& buffer, Image& image);
}
