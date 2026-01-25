#pragma once

#include <vulkan/vulkan.h>

class Descriptor {
public:
    VkDescriptorSet set;
    VkDescriptorPool pool;
    VkDescriptorSetLayout layout;
private:
    VkDevice device;
    VkDescriptorSetLayoutBinding layoutBinding{};
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    VkDescriptorSetAllocateInfo allocCreateInfo{};
    VkDescriptorPoolSize poolSize{};
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    VkWriteDescriptorSet descriptorWrite{};
public:
    Descriptor() = default;
    ~Descriptor();

    void init(VkDevice device, uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkImageView imageView, VkSampler sampler);
    void setDefaults();
    void prepareBuild();
private:
};