#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "ShaderStagesBuilder.hpp"

class GraphicsPipelineBuilder {
public:
private:
    VkDevice device;
    std::vector<VkDynamicState> dynamicStates{};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    std::vector<VkFormat> colorAttachmentFormats;
    VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
public:

    GraphicsPipelineBuilder();
    ~GraphicsPipelineBuilder();
    GraphicsPipelineBuilder(const GraphicsPipelineBuilder&) = delete;
    GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&) = delete;

    GraphicsPipelineBuilder& setDefaults();

    GraphicsPipelineBuilder& addColorFormat(VkFormat colorFormat);

    GraphicsPipelineBuilder& addColorFormats(const std::vector<VkFormat> &colorFormats);

    GraphicsPipelineBuilder& depthFormat(VkFormat depthFormat);

    void build(VkDevice device, VkPipelineLayout pipelineLayout, ShaderStagesBuilder& shaderBuilder, VkPipeline &pipeline);
private:
};