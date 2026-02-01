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
#include "Renderer/BarrierBuilder.hpp"
#include "Components/TerrainConfig.hpp"
#include "Renderer/ImageCopyBuilder.hpp"
#include "Components/NoiseGenerator.hpp"
#include "Components/HeightmapConfig.hpp"
#include "Components/ChunkIndicesGenerator.hpp"
#include "RHI/Pipeline/Descriptor/DescriptorSetBuilder.hpp"
#include "RHI/Pipeline/Initialization/PipelineLayoutBuilder.hpp"
#include "RHI/Pipeline/Initialization/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    constants = {};
    const std::string APP_NAME = "MeshApp";
    engine.init(APP_NAME, &constants.mvp);
    playerPos = {0, 0, 0};

    VkDevice device = engine.renderer.vulkanContext.device;
    VmaAllocator allocator = engine.renderer.vulkanContext.allocator;

    imageSystem = ImageSystem(device, allocator);

    chunkManager.init(&playerPos, allocator, imageSystem);
    chunkManager.updateChunkLinks();
    chunkManager.fillGpuBuffer();
    chunkManager.uploadChunkLinks(engine.renderer.vulkanContext);

    heightmapId = chunkManager.heightmapId;
    Image heightmap = imageSystem.get(heightmapId);

    createTerrainIndicesBuffer();
    createHeightmap();

    GraphicsPipelineBuilder gPipelineBuilder;
    gPipelineBuilder.addColorFormat(engine.renderer.swapchain.surfaceFormat.format);

    ShaderStagesBuilder shaderBuilder;
    shaderBuilder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "shaders/terrain.vert.spv")
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/terrain.frag.spv");

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    vkCreateSampler(device, &samplerInfo, nullptr, &heightmapSampler);

    DescriptorSetBuilder chunkSetBuilder;
    chunkSetBuilder.addTexture(
        TEXTURE_SAMPLER_BINDING,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        heightmap.imageView,
        heightmapSampler
    );
    chunkSetBuilder.addBuffer(
        CHUNK_DATA_BINDING,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        chunkManager.gpuBuffer
    );
    heightmapDescriptorSet.init(device,chunkSetBuilder);

    PipelineLayoutBuilder layoutBuilder;
    layoutBuilder.addPushConstants(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstants))
        .addDescriptorSet(heightmapDescriptorSet.layout);

    pipeline.init(device, layoutBuilder, shaderBuilder, gPipelineBuilder);
}

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    if (heightmapSampler) { vkDestroySampler(device, heightmapSampler, nullptr); }
}

void MeshApp::createTerrainIndicesBuffer() {
    std::vector<uint32_t> indices = ChunkIndicesGenerator::getIndices();
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

    VkImage heightmapImage = imageSystem.get(heightmapId).image;

    Buffer stagingBuffer;
    stagingBuffer.init(allocator, HeightmapConfig::HEIGHTMAP_SIZE, BufferType::STAGING_UPLOAD);
    stagingBuffer.upload(mockTerrain.data(), HeightmapConfig::HEIGHTMAP_SIZE);

    VkCommandBuffer cmd = vkCtx.singleTimeCmdBegin(transferQueueCtx);

    BarrierBuilder::onImage(
        heightmapImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    )
    .layerCount(TerrainConfig::INSTANCE_COUNT)
    .access(0, VK_ACCESS_TRANSFER_WRITE_BIT)
    .stages(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)
    .record(cmd);

    ImageCopyBuilder(
        stagingBuffer.buffer,
        heightmapImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        {TerrainConfig::RESOLUTION, TerrainConfig::RESOLUTION, 1}
    )
    .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
    .layerCount(TerrainConfig::INSTANCE_COUNT)
    .record(cmd);

    BarrierBuilder::onImage(
        heightmapImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    .layerCount(TerrainConfig::INSTANCE_COUNT)
    .stages(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) .access(VK_ACCESS_TRANSFER_WRITE_BIT, 0)
    .queues(transferQueueCtx, graphicsQueueCtx)
    .record(cmd);

    vkCtx.singleTimeCmdSubmit(transferQueueCtx, cmd);

    cmd = vkCtx.singleTimeCmdBegin(graphicsQueueCtx);
    BarrierBuilder::onImage(
        heightmapImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    .layerCount(TerrainConfig::INSTANCE_COUNT)
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
    vkCmdDrawIndexed(commandBuffer, ChunkIndicesGenerator::INDEX_COUNT, TerrainConfig::INSTANCE_COUNT, 0, 0, 0);
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}
