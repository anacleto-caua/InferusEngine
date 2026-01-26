#pragma once

#include <vector>

#include <vulkan/vulkan.h>

struct GraphicsPipelineBuilder {
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

    static GraphicsPipelineBuilder start();

    GraphicsPipelineBuilder& setDefaults();

    GraphicsPipelineBuilder& addColorFormat(VkFormat colorFormat);

    GraphicsPipelineBuilder& addColorFormats(const std::vector<VkFormat> &colorFormats);

    GraphicsPipelineBuilder& depthFormat(VkFormat depthFormat);

    GraphicsPipelineBuilder& addShaderStage(VkPipelineShaderStageCreateInfo info);

    VkPipeline build(VkDevice device, VkPipelineLayout pipelineLayout);
};