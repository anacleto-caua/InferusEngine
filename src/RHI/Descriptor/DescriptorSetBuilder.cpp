#include "DescriptorSetBuilder.hpp"

#include <stdexcept>

void DescriptorSetBuilder::addTexture(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage, VkImageView view, VkSampler sampler) {
    TextureConfig textureConfig{};
    textureConfig.binding = binding;
    textureConfig.type = type;
    textureConfig.stage = stage;
    textureConfig.view = view;
    textureConfig.sampler = sampler;
    configs.push_back(textureConfig);
}

void DescriptorSetBuilder::build(VkDevice device, VkDescriptorSet& set, VkDescriptorPool& pool, VkDescriptorSetLayout& layout) {
    std::vector<VkDescriptorImageInfo> imageInfos;

    std::vector<VkWriteDescriptorSet> writes(configs.size());
    std::vector<VkDescriptorSetLayoutBinding> bindings(configs.size());

    for (size_t i = 0; i < configs.size(); i++) {
        const TextureConfig& config = configs[i];

        VkDescriptorSetLayoutBinding setLayout{};
        setLayout.binding = config.binding;
        setLayout.descriptorType = config.type;
        setLayout.descriptorCount = 1;
        setLayout.stageFlags = config.stage;
        setLayout.pImmutableSamplers = nullptr;
        bindings[i] = setLayout;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = config.view;
        imageInfo.sampler = config.sampler;
        imageInfos.push_back(imageInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = config.binding;
        write.dstArrayElement = 0;
        write.descriptorType = config.type;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfos[i];
        writes[i] = write;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("descriptor set layout creation failed");
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const TextureConfig& config : configs) {
        poolSizes.push_back({config.type, 1});
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