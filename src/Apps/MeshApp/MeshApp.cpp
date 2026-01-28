#include "MeshApp.hpp"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RHI/Buffer.hpp"
#include "RHI/RHITypes.hpp"
#include "RHI/VulkanContext.hpp"
#include "Components/Heightmap.hpp"
#include "Renderer/BarrierBuilder.hpp"
#include "Components/NoiseGenerator.hpp"
#include "Components/TerrainChunkData.hpp"
#include "RHI/Pipeline/Descriptor/DescriptorSetBuilder.hpp"
#include "RHI/Pipeline/Initialization/PipelineLayoutBuilder.hpp"
#include "RHI/Pipeline/Initialization/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    constants = {};
    const std::string APP_NAME = "MeshApp";
    engine.init(APP_NAME, &constants.mvp);
    VkDevice device = engine.renderer.vulkanContext.device;

    createTerrainIndicesBuffer();
    createHeightmap();

    VmaAllocator allocator = engine.renderer.vulkanContext.allocator;
    size_t chunkDataSize = sizeof(TerrainChunkData::ChunkData) * TerrainChunkData::INSTANCE_COUNT;
    chunkDataHost.init(allocator, chunkDataSize, BufferType::STAGING_UPLOAD);
    chunkDataDevice.init(allocator, chunkDataSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    std::vector<TerrainChunkData::ChunkData> mockChunkData = TerrainChunkData::generateChunkGrid(TerrainChunkData::INSTANCE_COUNT);
    chunkDataHost.upload(mockChunkData.data(), chunkDataSize);
    chunkDataDevice.immediateCopy(engine.renderer.vulkanContext, chunkDataHost, chunkDataSize);

    GraphicsPipelineBuilder gPipelineBuilder;
    gPipelineBuilder.addColorFormat(engine.renderer.swapchain.surfaceFormat.format);

    ShaderStagesBuilder shaderBuilder;
    shaderBuilder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "shaders/terrain.vert.spv")
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/terrain.frag.spv");

    DescriptorSetBuilder chunkSetBuilder;
    chunkSetBuilder.addTexture(
        TEXTURE_SAMPLER_BINDING,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        heightmap.imageView,
        heightmap.sampler
    );
    chunkSetBuilder.addBuffer(
        CHUNK_DATA_BINDING,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        chunkDataDevice
    );
    heightmapDescriptorSet.init(device,chunkSetBuilder);

    PipelineLayoutBuilder layoutBuilder;
    layoutBuilder.addPushConstants(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstants))
        .addDescriptorSet(heightmapDescriptorSet.layout);

    pipeline.init(device, layoutBuilder, shaderBuilder, gPipelineBuilder);
    engine.renderer.pipeline = &pipeline.pipeline;
}

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    Heightmap::destroy(heightmap, device, engine.renderer.vulkanContext.allocator);
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
            this->drawCallback(cmd);
        });
    }
    vkDeviceWaitIdle(engine.renderer.vulkanContext.device);
}

void MeshApp::drawCallback(VkCommandBuffer commandBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, terrainIndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(
        commandBuffer,
        pipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PushConstants),
        &constants
    );

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.pipelineLayout,
        TEXTURE_SAMPLER_BINDING,
        1,
        &heightmapDescriptorSet.set,
        0,
        nullptr
    );
    vkCmdDrawIndexed(commandBuffer, TerrainChunkData::INDEX_COUNT, TerrainChunkData::INSTANCE_COUNT, 0, 0, 0);
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}
