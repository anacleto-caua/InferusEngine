#include "RHI/Recipes/CopyBufferToImageRecipes.hpp"

namespace CopyBufferToImageBuilder {
   VkBufferImageCopy DefaultCopy(Buffer& buffer, Image& image) {
        VkBufferImageCopy imageCopy {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = image.arrayLayers
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {image.width, image.height, 1}
        };
        return imageCopy;
   }
}
