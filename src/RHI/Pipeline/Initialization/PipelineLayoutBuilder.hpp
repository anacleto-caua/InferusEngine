#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class PipelineLayoutBuilder {
public:
private:
    VkPushConstantRange pushConstantRange{};
    VkPipelineLayoutCreateInfo layoutCreateInfo{};
    std::vector<VkDescriptorSetLayout> setLayouts;
public:
    static PipelineLayoutBuilder start();
    PipelineLayoutBuilder& addPushConstants(VkShaderStageFlags stageFlags, size_t size);
    PipelineLayoutBuilder& addDescriptorSet(VkDescriptorSetLayout setLayout);
    void build(VkDevice device, VkPipelineLayout& pipelineLayout);
private:
    void setDefaults();
};