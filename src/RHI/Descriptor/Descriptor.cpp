#include "Descriptor.hpp"

void Descriptor::init(VkDevice device, uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkImageView imageView, VkSampler sampler) {
    this->device = device;
    setDefaults();

    layoutBinding.binding = binding;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.descriptorType = descriptorType;
    poolSize.type = descriptorType;
    descriptorWrite.descriptorType = descriptorType;

    prepareBuild();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    descriptorWrite.pImageInfo = &imageInfo;

    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = layoutBinding.binding;
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

Descriptor::~Descriptor() {
    if (pool) vkDestroyDescriptorPool(device, pool, nullptr);
    if (layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

void Descriptor::setDefaults() {
    layoutBinding.descriptorCount = 1;
    layoutBinding.pImmutableSamplers = nullptr;

    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;

    poolSize.descriptorCount = 1;

    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.maxSets = 1;

    allocCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocCreateInfo.descriptorSetCount = 1;

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;

}

void Descriptor::prepareBuild() {
    layoutCreateInfo.pBindings = &layoutBinding;
    vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &layout);
    poolCreateInfo.pPoolSizes = &poolSize;
    vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &pool);
    allocCreateInfo.descriptorPool = pool;
    allocCreateInfo.pSetLayouts = &layout;
    vkAllocateDescriptorSets(device, &allocCreateInfo, &set);
}