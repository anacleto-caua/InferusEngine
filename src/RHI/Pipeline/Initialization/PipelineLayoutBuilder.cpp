#include "PipelineLayoutBuilder.hpp"

PipelineLayoutBuilder PipelineLayoutBuilder::start() {
    PipelineLayoutBuilder builder;
    builder.setDefaults();
    return builder;
}

void PipelineLayoutBuilder::setDefaults() {
    pushConstantRange.stageFlags = 0;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 0;

    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 0;
    layoutCreateInfo.pSetLayouts = VK_NULL_HANDLE;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::addDescriptorSet(VkDescriptorSetLayout setLayout) {
    setLayouts.push_back(setLayout);
    return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::addPushConstants(VkShaderStageFlags stageFlags, size_t size) {
    pushConstantRange.stageFlags = stageFlags;
    pushConstantRange.offset = 0;
    pushConstantRange.size = size;

    layoutCreateInfo.pushConstantRangeCount = 1;
    return *this;
}

void PipelineLayoutBuilder::build(VkDevice device, VkPipelineLayout& pipelineLayout) {
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = setLayouts.data();

    layoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout);
}