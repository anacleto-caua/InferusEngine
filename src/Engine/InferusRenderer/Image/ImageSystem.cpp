#include "ImageSystem.hpp"

#include <cstdint>

#include "Engine/InferusRenderer/RendererConfig.hpp"

void ImageSystem::create(VkDevice VkDevice, VmaAllocator VmaAllocator) {
    this->Device = VkDevice;
    this->Allocator = VmaAllocator;

    Data.clear();
    Data.reserve(RendererConfig::ImageSystem::DATA_RESERVE_CAPACITY);
    FreeIndices.clear();
    FreeIndices.reserve(RendererConfig::ImageSystem::FREE_INDICES_RESERVE_CAPACITY);
}

void ImageSystem::destroy() {
    for (auto image: Data) {
        destroy(image);
    }
}

ImageId ImageSystem::add(ImageCreateDescription imageDesc) {
    ImageId id;
    Image image{};

    if(FreeIndices.empty()) {
        id = { .index = (uint32_t)Data.size() };
        Data.resize(Data.size() + 1);
    } else {
        id = { FreeIndices.back() };
        FreeIndices.pop_back();
    }

    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.mipLevels = 1;
    createInfo.extent.depth = 1;
    createInfo.format = imageDesc.format;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.extent.width = imageDesc.width;
    createInfo.extent.height = imageDesc.height;
    createInfo.arrayLayers = imageDesc.arrayLayers;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateImage(Allocator, &createInfo, &allocCreateInfo, &image.image, &image.allocation, nullptr);

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image.image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    imageViewCreateInfo.format = imageDesc.format;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = imageDesc.arrayLayers;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    vkCreateImageView(Device, &imageViewCreateInfo, nullptr, &image.imageView);

    image.width = imageDesc.width;
    image.height = imageDesc.height;
    image.format = imageDesc.format;
    image.mipLevels = imageDesc.mipLevels;
    image.arrayLayers = imageDesc.arrayLayers;
    image.depth = imageDesc.depth;
    image.format = imageDesc.format;
    image.layout = VK_IMAGE_LAYOUT_UNDEFINED;

    Data[id.index] = image;
    return id;
}

Image& ImageSystem::get(ImageId id) {
    return Data[id.index];
}

void ImageSystem::del(ImageId id) {
    destroy(Data[id.index]);
    FreeIndices.push_back(id);
}

void ImageSystem::upload(ImageId id, void *upload_data, size_t size) {
    // Mock usage to avoid compiler warnings
    (void)id;
    (void)upload_data;
    (void)size;
}

void ImageSystem::destroy(Image& image) {
    vkDestroyImageView(Device, image.imageView, nullptr);
    vmaDestroyImage(Allocator, image.image, image.allocation);
    image.image = VK_NULL_HANDLE;
    image.imageView = VK_NULL_HANDLE;
}
