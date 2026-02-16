#include "DescriptorSet.hpp"

DescriptorSet::~DescriptorSet() {
    if (pool) vkDestroyDescriptorPool(device, pool, nullptr);
    if (layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

void DescriptorSet::init(VkDevice vk_device, DescriptorSetBuilder &builder) {
    this->device = vk_device;
    builder.build(device, set, pool, layout);
}
