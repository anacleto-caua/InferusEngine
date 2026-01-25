#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class DescriptorSetBuilder {
public:
    struct TextureConfig {
        uint32_t binding;
        VkDescriptorType type;
        VkShaderStageFlags stage;
        VkImageView view;
        VkSampler sampler;
    };

    std::vector<TextureConfig> configs;
private:
public:
    DescriptorSetBuilder() = default;
    ~DescriptorSetBuilder() = default;

    void addTexture(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkImageView view, VkSampler sampler);
    void build(VkDevice device, VkDescriptorSet& set, VkDescriptorPool& pool, VkDescriptorSetLayout& layout);
};