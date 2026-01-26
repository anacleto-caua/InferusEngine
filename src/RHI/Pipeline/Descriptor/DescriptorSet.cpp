#include "DescriptorSet.hpp"

DescriptorSet::~DescriptorSet() {
    if (pool) vkDestroyDescriptorPool(device, pool, nullptr);
    if (layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

void DescriptorSet::init(VkDevice device, DescriptorSetBuilder &builder) {
    this->device = device;
    builder.build(device, set, pool, layout);
}