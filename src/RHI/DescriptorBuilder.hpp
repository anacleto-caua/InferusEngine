#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

class DescriptorBuilder {
public:
    VkDescriptorSetLayoutBinding layoutBinding{};
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    VkDescriptorSetAllocateInfo allocInfo{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
private:
public:
    static DescriptorBuilder begin(VkDevice device, uint32_t layoutBinding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {
        DescriptorBuilder builder;
        builder.layoutBinding.binding = layoutBinding;
        builder.layoutBinding.descriptorType = descriptorType;
        builder.layoutBinding.stageFlags = stageFlags;
        builder.setDefaults(device);
        return builder;
    }

    VkDescriptorSet buildTexture(VkDevice device, VkImageView imageView, VkSampler sampler) {
        VkDescriptorSet descriptorSet;
        vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = layoutBinding.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        return descriptorSet;
    }
private:
    void setDefaults(VkDevice device) {
        layoutBinding.descriptorCount = 1;
        layoutBinding.pImmutableSamplers = nullptr;

        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &layoutBinding;

        vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;
    }
};