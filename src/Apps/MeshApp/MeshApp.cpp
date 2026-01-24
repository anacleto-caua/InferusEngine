#include "MeshApp.hpp"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Components/Heightmap.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/RHITypes.hpp"
#include "RHI/VulkanContext.hpp"
#include "Renderer/BarrierBuilder.hpp"
#include "Components/NoiseGenerator.hpp"
#include "Components/TerrainChunkData.hpp"
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
    createHeightmap();
}

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    Heightmap::destroy(heightmap, device, engine.renderer.vulkanContext.allocator);
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}

void MeshApp::createTerrainIndicesBuffer() {
    std::vector<uint32_t> indices = TerrainChunkData::getIndices();
    uint32_t bufferSize = indices.size() * sizeof(uint32_t);
    terrainIndicesBuffer.init(engine.renderer.vulkanContext.allocator, bufferSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    terrainIndicesBuffer.immediateUpload(engine.renderer.vulkanContext, indices.data(), bufferSize);
}

void MeshApp::createHeightmap() {
    VulkanContext& vkCtx = engine.renderer.vulkanContext;
    VmaAllocator allocator = vkCtx.allocator;
    QueueContext& transferQueueCtx = vkCtx.transferQueueCtx;
    QueueContext& graphicsQueueCtx = vkCtx.graphicsQueueCtx;
    std::vector<uint16_t> mockTerrain = NoiseGenerator::generateMockHeightmaps();

    heightmap = Heightmap::create(vkCtx.device, allocator);

    Buffer stagingBuffer;
    stagingBuffer.init(allocator, Heightmap::HEIGHTMAP_SIZE, BufferType::STAGING_UPLOAD);
    stagingBuffer.upload(mockTerrain.data(), Heightmap::HEIGHTMAP_SIZE);

    VkCommandBuffer cmd = vkCtx.singleTimeCmdBegin(transferQueueCtx);

    BarrierBuilder::onImage(
        heightmap.image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    )
    .layerCount(TerrainChunkData::INSTANCE_COUNT)
    .access(0, VK_ACCESS_TRANSFER_WRITE_BIT)
    .stages(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)
    .record(cmd);

    VkBufferImageCopy imageCopy{};
    imageCopy.bufferOffset = 0;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;
    imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopy.imageSubresource.mipLevel = 0;
    imageCopy.imageSubresource.baseArrayLayer = 0;
    imageCopy.imageSubresource.layerCount = TerrainChunkData::INSTANCE_COUNT;
    imageCopy.imageOffset = {0, 0, 0};
    imageCopy.imageExtent = {TerrainChunkData::RESOLUTION, TerrainChunkData::RESOLUTION, 1};

    vkCmdCopyBufferToImage(
        cmd,
        stagingBuffer.buffer,
        heightmap.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopy
    );

    BarrierBuilder::onImage(
        heightmap.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    .layerCount(TerrainChunkData::INSTANCE_COUNT)
    .stages(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
    .access(VK_ACCESS_TRANSFER_WRITE_BIT, 0)
    .queues(transferQueueCtx, graphicsQueueCtx)
    .record(cmd);

    vkCtx.singleTimeCmdSubmit(transferQueueCtx, cmd);

    cmd = vkCtx.singleTimeCmdBegin(graphicsQueueCtx);
    BarrierBuilder::onImage(
        heightmap.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    .layerCount(TerrainChunkData::INSTANCE_COUNT)
    .stages(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
    .access(0, VK_ACCESS_SHADER_READ_BIT)
    .queues(transferQueueCtx, graphicsQueueCtx)
    .record(cmd);

    vkCtx.singleTimeCmdSubmit(graphicsQueueCtx, cmd);
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
        vkCmdDrawIndexed(commandBuffer, TerrainChunkData::INDEX_COUNT, TerrainChunkData::INSTANCE_COUNT, 0, 0, 0);
    }
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}