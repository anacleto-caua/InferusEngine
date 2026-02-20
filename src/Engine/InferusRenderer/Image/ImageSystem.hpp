#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "Engine/InferusRenderer/Image/Image.hpp"
#include "Engine/InferusRenderer/Image/ImageCreateDescription.hpp"

class ImageSystem {
public:
private:
    VkDevice Device;
    VmaAllocator Allocator;
    std::vector<Image> Data;
    std::vector<ImageId> FreeIndices;
public:
    ImageSystem() = default;
    ~ImageSystem() = default;
    ImageSystem(const ImageSystem&) = delete;
    ImageSystem& operator=(const ImageSystem&) = delete;

    void create(VkDevice VkDevice, VmaAllocator VmaAllocator);
    void destroy();

    ImageId add(ImageCreateDescription imageCreateDesc);
    Image& get(ImageId id);
    void del(ImageId id);
    void upload(ImageId id, void *data, size_t size);
private:
    void destroy(Image& image);
};
