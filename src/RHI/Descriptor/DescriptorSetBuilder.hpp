#pragma once

#include <deque>
#include <vector>

#include <vulkan/vulkan.h>

#include "RHI/Buffer.hpp"

class DescriptorSetBuilder {
public:
    struct BindConfig {
        uint32_t binding;
        VkDescriptorType type;
        VkShaderStageFlags stage;
    };

    struct TextureConfig {
        BindConfig bind;
        VkDescriptorImageInfo imageInfo{};
    };

    struct BufferConfig {
        BindConfig bind;
        VkDescriptorBufferInfo bufferInfo{};
    };

    std::deque<TextureConfig> textureConfigs;
    std::deque<BufferConfig> bufferConfigs;

private:
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
public:
    DescriptorSetBuilder() = default;
    ~DescriptorSetBuilder() = default;

    void addTexture(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkImageView view, VkSampler sampler);
    void addBuffer(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, Buffer buffer);
    void build(VkDevice device, VkDescriptorSet& set, VkDescriptorPool& pool, VkDescriptorSetLayout& layout);
private:
    void addBuffer(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    void bind(BindConfig config);
};