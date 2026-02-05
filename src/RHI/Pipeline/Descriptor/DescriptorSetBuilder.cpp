#include "DescriptorSetBuilder.hpp"

#include <map>
#include <stdexcept>

void DescriptorSetBuilder::addTexture(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkImageView view, VkSampler sampler) {
    TextureConfig config{};
    config.bind.binding = binding;
    config.bind.type = type;
    config.bind.stage = stage;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = view;
    imageInfo.sampler = sampler;

    config.imageInfo = imageInfo;
    textureConfigs.push_back(config);
}

const void DescriptorSetBuilder::addBuffer(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, const Buffer buffer) {
    addBuffer(binding, type, stage, buffer.buffer, 0, buffer.size);
}

void DescriptorSetBuilder::addBuffer(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
    BufferConfig config{};
    config.bind.binding = binding;
    config.bind.type = type;
    config.bind.stage = stage;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    config.bufferInfo = bufferInfo;
    bufferConfigs.push_back(config);
}

void DescriptorSetBuilder::bind(BindConfig config) {
    VkDescriptorSetLayoutBinding setLayout{};
    setLayout.binding = config.binding;
    setLayout.descriptorType = config.type;
    setLayout.descriptorCount = 1;
    setLayout.stageFlags = config.stage;
    setLayout.pImmutableSamplers = nullptr;
    bindings.push_back(setLayout);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = config.binding;
    write.dstArrayElement = 0;
    write.descriptorType = config.type;
    write.descriptorCount = 1;
    writes.push_back(write);
}

void DescriptorSetBuilder::build(VkDevice device, VkDescriptorSet& set, VkDescriptorPool& pool, VkDescriptorSetLayout& layout) {
    size_t setCount = bufferConfigs.size() + textureConfigs.size();

    for (const TextureConfig &config : textureConfigs) {
        bind(config.bind);
        writes.back().pImageInfo = &config.imageInfo;
    }

    for (const BufferConfig &config : bufferConfigs) {
        bind(config.bind);
        writes.back().pBufferInfo = &config.bufferInfo;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("descriptor set layout creation failed");
    }

    std::map<VkDescriptorType, uint32_t> poolSizeMap;
    for (const TextureConfig& config : textureConfigs) {
        poolSizeMap[config.bind.type]++;
    }
    for (const BufferConfig& config : bufferConfigs) {
        poolSizeMap[config.bind.type]++;
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& [type, count] : poolSizeMap) {
        poolSizes.push_back({type, count});
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("descriptor pool creation failed");
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
        throw std::runtime_error("descriptor set allocation failed");
    }

    for (VkWriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
