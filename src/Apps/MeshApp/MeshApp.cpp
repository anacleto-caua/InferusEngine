#include "MeshApp.hpp"

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Apps/MeshApp/AppTypes.hpp"
#include "RHI/Pipeline/ShaderStageBuilder.hpp"
#include "RHI/Pipeline/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    const std::string APP_NAME = "MeshApp";
    engine.init(APP_NAME);
    VkDevice device = engine.renderer.vulkanContext.device;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

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

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}

void MeshApp::run() {
    while (!shouldClose()) {
        engine.update();
        engine.render([this](VkCommandBuffer cmd) {
            MeshApp::drawCallback(cmd, this);
        });
    }
    vkDeviceWaitIdle(engine.renderer.vulkanContext.device);
}

void MeshApp::drawCallback(VkCommandBuffer commandBuffer, MeshApp* app) {
    if (app->pipeline != VK_NULL_HANDLE) {
        static float angle = 0.0f;
        angle += 0.00001f;

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0,0,1));

        PushConstants constants;
        constants.renderMatrix = model;
        constants.data = glm::vec4(angle, 0, 0, 0);

        vkCmdPushConstants(
            commandBuffer,
            app->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstants),
            &constants
        );

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}