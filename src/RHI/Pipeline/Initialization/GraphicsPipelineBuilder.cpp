#include "GraphicsPipelineBuilder.hpp"

#include <cstdint>
#include <stdexcept>

#include "ShaderStagesBuilder.hpp"

GraphicsPipelineBuilder::GraphicsPipelineBuilder() {
    setDefaults();
}

GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {
    for (VkPipelineShaderStageCreateInfo stage : shaderStages) {
        if(stage.module) { vkDestroyShaderModule(device, stage.module, nullptr); }
    }
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDefaults() {
    dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 0;
    dynamicState.pDynamicStates = nullptr;

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = .2f;

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    colorAttachmentFormats.clear();
    depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorFormat(VkFormat colorFormat) {
    colorAttachmentFormats.push_back(colorFormat);
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorFormats(const std::vector<VkFormat> &colorFormats) {
    colorAttachmentFormats.insert(colorAttachmentFormats.end(), colorFormats.begin(), colorFormats.end());
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::depthFormat(VkFormat depthFormat) {
    depthAttachmentFormat = depthFormat;
    return *this;
}

void GraphicsPipelineBuilder::build(VkDevice vk_device, VkPipelineLayout pipelineLayout, ShaderStagesBuilder& shaderBuilder, VkPipeline &pipeline) {
    this->device = vk_device;
    shaderBuilder.build(device, shaderStages);

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineColorBlendAttachmentState defaultColorBlendState{};
    defaultColorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    defaultColorBlendState.blendEnable = VK_TRUE;
    defaultColorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    defaultColorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    defaultColorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
    defaultColorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    defaultColorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    defaultColorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachmentFormats.size(), defaultColorBlendState);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    colorBlending.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
    colorBlending.pAttachments = blendAttachments.data();

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentFormats.size());
    renderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
    renderingInfo.depthAttachmentFormat = depthAttachmentFormat;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.pNext = &renderingInfo;

    // Assign all pointers fresh
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    (void)colorBlendAttachment; // To silence compiler warnings

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("graphics pipeline creation failed");
    }
}
