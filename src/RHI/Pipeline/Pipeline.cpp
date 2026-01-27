#include "Pipeline.hpp"

void Pipeline::init(VkDevice device, PipelineLayoutBuilder& layoutBuilder, ShaderStagesBuilder& shaderBuilder, GraphicsPipelineBuilder& gPipelineBuilder) {
    this->device = device;

    layoutBuilder.build(device, pipelineLayout);
    gPipelineBuilder.build(device, pipelineLayout, shaderBuilder, pipeline);
}

Pipeline::~Pipeline() {
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}