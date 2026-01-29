#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

class ImageCopyBuilder {
public:
private:
    VkBufferImageCopy imageCopy{};
    VkExtent3D extent{};
    VkBuffer buffer;
    VkImage image;
    VkImageLayout layout;
    uint32_t regionCount;

public:
    ImageCopyBuilder(VkBuffer buffer, VkImage image, VkImageLayout layout, VkExtent3D extent) {
        this->buffer = buffer;
        this->image = image;
        this->layout = layout;
        imageCopy.imageExtent = extent;

        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        imageCopy.imageSubresource.aspectMask = 0;
        imageCopy.imageSubresource.mipLevel = 0;
        imageCopy.imageSubresource.baseArrayLayer = 0;
        imageCopy.imageSubresource.layerCount = 0;
        imageCopy.imageOffset = {0, 0, 0};

        regionCount = 1;
    }

    ImageCopyBuilder& aspectMask(VkImageAspectFlags aspectMask) {
        imageCopy.imageSubresource.aspectMask = aspectMask;
        return *this;
    }

    ImageCopyBuilder& layerCount(uint32_t layerCount) {
        imageCopy.imageSubresource.layerCount = layerCount;
        return *this;
    }

    void record(VkCommandBuffer &cmd) {
        vkCmdCopyBufferToImage(
            cmd,
            buffer,
            image,
            layout,
            1,
            &imageCopy
        );
    }
private:
};