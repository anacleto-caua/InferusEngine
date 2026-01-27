#pragma once

#include <vector>

#include "RHI/Pipeline/Descriptor/DescriptorSet.hpp"
#include "RHI/Pipeline/Initialization/ShaderStagesBuilder.hpp"
#include "RHI/Pipeline/Initialization/PipelineLayoutBuilder.hpp"
#include "RHI/Pipeline/Initialization/GraphicsPipelineBuilder.hpp"

class Pipeline {
public:
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<DescriptorSet> sets;
    const void* pConstants;
private:
    VkDevice device;
public:
    Pipeline() = default;
    ~Pipeline();
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void init(VkDevice device, PipelineLayoutBuilder& layoutBuilder, ShaderStagesBuilder& shaderBuilder, GraphicsPipelineBuilder& gPipelineBuilder);

private:
};