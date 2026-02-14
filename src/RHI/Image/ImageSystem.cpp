#include <cstdint>

#include "RHI/RHIConfig.hpp"
#include "RHI/Image/ImageSystem.hpp"

void ImageSystem::init(VkDevice vk_device, VmaAllocator vma_allocator) {
    this->device = vk_device;
    this->allocator = vma_allocator;

    data.clear();
    data.reserve(RHIConfig::ImageSystemConfig::ImageReserveCapacity);
    freeIndices.clear();
    freeIndices.reserve(RHIConfig::ImageSystemConfig::FreeIndicesReserveCapacity);
}

ImageSystem::~ImageSystem() {
    for (auto image: data) {
        destroy(image);
    }
}

ImageId ImageSystem::add(ImageCreateDescription imageDesc) {
    ImageId id;
    Image image{};

    if(freeIndices.empty()) {
        id = { .index = (uint32_t)data.size() };
        data.resize(data.size() + 1);
    } else {
        id = { .index = freeIndices.back() };
        freeIndices.pop_back();
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

    vmaCreateImage(allocator, &createInfo, &allocCreateInfo, &image.image, &image.allocation, nullptr);

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

    vkCreateImageView(device, &imageViewCreateInfo, nullptr, &image.imageView);

    image.width = imageDesc.width;
    image.height = imageDesc.height;
    image.format = imageDesc.format;
    image.mipLevels = imageDesc.mipLevels;
    image.arrayLayers = imageDesc.arrayLayers;
    image.depth = imageDesc.depth;
    image.format = imageDesc.format;
    image.layout = VK_IMAGE_LAYOUT_UNDEFINED;

    data[id.index] = image;
    return id;
}

Image& ImageSystem::get(ImageId id) {
    return data[id.index];
}

void ImageSystem::del(ImageId id) {
    destroy(data[id.index]);
    freeIndices.push_back(id.index);
}

void ImageSystem::upload(ImageId id, void *upload_data, size_t size) {
    // Mock usage to avoid compiler warnings
    (void)id;
    (void)upload_data;
    (void)size;
}

void ImageSystem::destroy(Image image) {
    vkDestroyImageView(device, image.imageView, nullptr);
    vmaDestroyImage(allocator, image.image, image.allocation);
}
