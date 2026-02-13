#include "MeshApp.hpp"

#include <array>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RHI/RHITypes.hpp"
#include "RHI/VulkanContext.hpp"
#include "Components/TerrainConfig.hpp"
#include "Components/HeightmapConfig.hpp"
#include "RHI/Recipes/BarrierRecipes.hpp"
#include "Components/ChunkIndicesGenerator.hpp"
#include "RHI/Recipes/CopyBufferToImageRecipes.hpp"
#include "RHI/Pipeline/Descriptor/DescriptorSetBuilder.hpp"
#include "RHI/Pipeline/Initialization/PipelineLayoutBuilder.hpp"
#include "RHI/Pipeline/Initialization/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    constants = {};
    engine.init(APP_NAME.data(), &constants.mvp);
    playerPos = {0, 0, 0};

    VkDevice device = engine.renderer.vulkanContext.device;
    VmaAllocator allocator = engine.renderer.vulkanContext.allocator;

    imageSystem.init(device, allocator);
    bufferManager.init(allocator);

    chunkManager.init(&playerPos, allocator, imageSystem, bufferManager);
    chunkManager.diamondUpdateChunkLinks();
    chunkManager.uploadChunkLinks(bufferManager, engine.renderer.vulkanContext);

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
        bufferManager.get(chunkManager.gpuBufferId)
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
    BufferCreateDescription terrainIndicesBufferCreateDesc {
        .size = bufferSize,
        .memType = BufferMemoryType::GPU_STATIC,
        .usage = BufferUsage::INDEX
    };

    terrainIndicesBufferId = bufferManager.add(terrainIndicesBufferCreateDesc);
    bufferManager.immediateUpload(engine.renderer.vulkanContext, terrainIndicesBufferId, indices.data(), bufferSize);
}

void MeshApp::createHeightmap() {
    VulkanContext& vkCtx = engine.renderer.vulkanContext;
    VmaAllocator allocator = vkCtx.allocator;
    QueueContext& transferQueueCtx = vkCtx.transferQueueCtx;
    QueueContext& graphicsQueueCtx = vkCtx.graphicsQueueCtx;
    auto mockTerrain = chunkManager.genHeightmap();

    Image heightmapImage = imageSystem.get(heightmapId);

    BufferCreateDescription stagingBufferCreateDesc {
        .size = HeightmapConfig::HEIGHTMAP_SIZE,
        .memType = BufferMemoryType::STAGING_UPLOAD,
        .usage = BufferUsage::STAGING
    };

    BufferId stagingBufferId = bufferManager.add(stagingBufferCreateDesc);
    bufferManager.upload(stagingBufferId, mockTerrain.data());

    VkCommandBuffer cmd = vkCtx.singleTimeCmdBegin(transferQueueCtx);

    VkImageMemoryBarrier barrier1 = BarrierRecipes::TransferDest(heightmapImage);
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier1
    );

    Buffer stagingBuffer = bufferManager.get(stagingBufferId);
    VkBufferImageCopy imageCopy = CopyBufferToImageBuilder::DefaultCopy(bufferManager.get(stagingBufferId), heightmapImage);

    vkCmdCopyBufferToImage(
        cmd,
        stagingBuffer.buffer,
        heightmapImage.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopy
    );

    VkImageMemoryBarrier barrier2 = BarrierRecipes::ShaderRead(heightmapImage);
    barrier2.srcQueueFamilyIndex = transferQueueCtx.index;
    barrier2.dstQueueFamilyIndex = graphicsQueueCtx.index;
    barrier2.dstAccessMask = 0;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier2
    );

    vkCtx.singleTimeCmdSubmit(transferQueueCtx, cmd);

    cmd = vkCtx.singleTimeCmdBegin(graphicsQueueCtx);

    VkImageMemoryBarrier barrier3 = BarrierRecipes::ShaderRead(heightmapImage);
    barrier3.srcQueueFamilyIndex = transferQueueCtx.index;
    barrier3.dstQueueFamilyIndex = graphicsQueueCtx.index;
    barrier3.srcAccessMask = 0;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier3
    );

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
    vkCmdBindIndexBuffer(commandBuffer, bufferManager.get(terrainIndicesBufferId).buffer, 0, VK_INDEX_TYPE_UINT32);

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
