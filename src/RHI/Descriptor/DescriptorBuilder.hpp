#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

class DescriptorBuilder {
public:
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
private:
    VkDescriptorSetLayoutBinding layoutBinding{};
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    VkDescriptorSetAllocateInfo allocCreateInfo{};
    VkDescriptorPoolSize poolSize{};
    VkDescriptorPoolCreateInfo poolCreateInfo{};
public:
    static DescriptorBuilder begin(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {
        DescriptorBuilder builder;
        builder.layoutBinding.binding = binding;
        builder.layoutBinding.descriptorType = descriptorType;
        builder.layoutBinding.stageFlags = stageFlags;
        builder.setDefaults();
        return builder;
    }

    DescriptorBuilder& buildSetTexture(VkDevice device, VkImageView imageView, VkSampler sampler) {
        prepareBuild(device);

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
        return *this;
    }

    void fetchObjects(VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorPool& descriptorPool, VkDescriptorSet& descriptorSet) {
        descriptorSetLayout = this->descriptorSetLayout;
        descriptorPool = this->descriptorPool;
        descriptorSet = this->descriptorSet;
    }

private:
    void setDefaults() {
        layoutBinding.descriptorCount = 1;
        layoutBinding.pImmutableSamplers = nullptr;

        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 1;

        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 1;
        poolCreateInfo.maxSets = 1;

        allocCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocCreateInfo.descriptorSetCount = 1;
    }

    void prepareBuild(VkDevice device) {
        layoutCreateInfo.pBindings = &layoutBinding;
        vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);
        poolCreateInfo.pPoolSizes = &poolSize;
        vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool);
        allocCreateInfo.descriptorPool = descriptorPool;
        allocCreateInfo.pSetLayouts = &descriptorSetLayout;
        vkAllocateDescriptorSets(device, &allocCreateInfo, &descriptorSet);
    }
};