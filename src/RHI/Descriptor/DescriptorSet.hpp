#pragma once

#include <vulkan/vulkan.h>

#include "DescriptorSetBuilder.hpp"

class DescriptorSet {
public:
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
private:
    VkDevice device = VK_NULL_HANDLE;
public:
    DescriptorSet() = default;
    ~DescriptorSet();
    void init(VkDevice device, DescriptorSetBuilder builder);
};