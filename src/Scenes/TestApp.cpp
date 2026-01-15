#include "TestApp.hpp"

#include <spdlog/spdlog.h>

#include "RHI/Pipeline/ShaderStageBuilder.hpp"
#include "RHI/Pipeline/GraphicsPipelineBuilder.hpp"

void TestApp::init() {
    const std::string APP_NAME = "TestApp";
    engine.init(APP_NAME);
    VkDevice device = engine.renderer.vulkanContext.device;

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    GraphicsPipelineBuilder builder = GraphicsPipelineBuilder::start()
    .setDefaults()
    .addColorFormat(engine.renderer.swapchain.surfaceFormat.format)
    .addShaderStage(
        ShaderStageBuilder::createShaderStage(
            device,
            VK_SHADER_STAGE_VERTEX_BIT,
            "shaders/base.vert.spv"
        )
    )
    .addShaderStage(
        ShaderStageBuilder::createShaderStage(
            device,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            "shaders/base.frag.spv"
        )
    );

    pipeline = builder.build(device, pipelineLayout);
    engine.renderer.pipeline = &pipeline;

    for (VkPipelineShaderStageCreateInfo shaderStage : builder.shaderStages) {
        if (shaderStage.module) { vkDestroyShaderModule(device, shaderStage.module, nullptr); }
    }
}

TestApp::~TestApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}

void TestApp::run() {
    while (!shouldClose()) {
        engine.update();
        engine.render([this](VkCommandBuffer cmd) {
            TestApp::drawCallback(cmd, this);
        });
    }
    vkDeviceWaitIdle(engine.renderer.vulkanContext.device);
}

void TestApp::drawCallback(VkCommandBuffer commandBuffer, TestApp* app) {
    if (app->pipeline != VK_NULL_HANDLE) {
        VkExtent2D extent = app->engine.renderer.swapchain.extent;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
}

bool TestApp::shouldClose() {
    return engine.shouldClose();
}