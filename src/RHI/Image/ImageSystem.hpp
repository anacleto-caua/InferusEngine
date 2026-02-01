#pragma once

#include <vector>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "RHI/Image/Image.hpp"
#include "RHI/Image/ImageCreateDescription.hpp"

class ImageSystem {
private:
    VkDevice device;
    VmaAllocator allocator;
    std::vector<Image> data;
    std::vector<uint32_t> freeIndices;
public:
    ImageSystem();
    ImageSystem(VkDevice device, VmaAllocator allocator);
    ~ImageSystem();

    ImageId add(ImageCreateDescription imageCreateDesc);
    Image& get(ImageId id);
    void del(ImageId id);
    void upload(ImageId id, void *data, size_t size);
private:
    void destroy(Image image);
};
