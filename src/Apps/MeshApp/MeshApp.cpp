#include "MeshApp.hpp"

#include <cstdint>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "RHI/Buffer.hpp"
#include "TerrainChunkData.hpp"
#include "RHI/Pipeline/ShaderStageBuilder.hpp"
#include "RHI/Pipeline/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    constants = {};
    const std::string APP_NAME = "MeshApp";
    engine.init(APP_NAME, &constants.mvp);
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
            "shaders/terrain.vert.spv"
        )
    )
    .addShaderStage(
        ShaderStageBuilder::createShaderStage(
            device,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            "shaders/terrain.frag.spv"
        )
    );

    pipeline = builder.build(device, pipelineLayout);
    engine.renderer.pipeline = &pipeline;

    for (VkPipelineShaderStageCreateInfo shaderStage : builder.shaderStages) {
        if (shaderStage.module) { vkDestroyShaderModule(device, shaderStage.module, nullptr); }
    }

    createTerrainIndicesBuffer();
}

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}

void MeshApp::createTerrainIndicesBuffer() {
    std::vector<uint32_t> indices = TerrainChunkData::getIndices();
    uint32_t bufferSize = indices.size() * sizeof(uint32_t);
    terrainIndicesBuffer.init(engine.renderer.vulkanContext.allocator, bufferSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    terrainIndicesBuffer.immediateUpload(engine.renderer.vulkanContext, indices.data(), bufferSize);
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
        vkCmdBindIndexBuffer(commandBuffer, app->terrainIndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdPushConstants(
            commandBuffer,
            app->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstants),
            &app->constants
        );

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);
        vkCmdDrawIndexed(commandBuffer, TerrainChunkData::INDEX_COUNT, 1, 0, 0, 0);
    }
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}