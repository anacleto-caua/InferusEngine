#include "Descriptor.hpp"

#include "DescriptorBuilder.hpp"

void Descriptor::init(VkDevice device, uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkImageView imageView, VkSampler sampler) {
    this->device = device;
    DescriptorBuilder::begin(binding, descriptorType, stageFlags)
        .buildSetTexture(device, imageView, sampler)
        .fetchObjects(layout, pool, set);
}

Descriptor::~Descriptor() {
    if (pool) vkDestroyDescriptorPool(device, pool, nullptr);
    if (layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);
}