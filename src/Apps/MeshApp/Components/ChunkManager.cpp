#include "ChunkManager.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "RHI/Image/ImageSystem.hpp"
#include "RHI/Image/ImageCreateDescription.hpp"
#include "Apps/MeshApp/Components/TerrainConfig.hpp"
#include "Apps/MeshApp/Components/NoiseGenerator.hpp"
#include "Apps/MeshApp/Components/HeightmapConfig.hpp"

void ChunkManager::init(glm::vec3* pPlayerPos,  VmaAllocator allocator, ImageSystem& imageSystem, BufferManager& bufferManager) {
   this->pPlayerPos = pPlayerPos;

    chunkLinks.resize(TerrainConfig::INSTANCE_COUNT);

    BufferCreateDescription cpuBufferCreateDesc{
        .size = chunkLinksSize,
        .memType = BufferMemoryType::STAGING_UPLOAD,
        .usage = BufferUsage::STAGING
    };
    cpuBufferId = bufferManager.add(cpuBufferCreateDesc);

    BufferCreateDescription gpuBufferCreateDesc{
        .size = chunkLinksSize,
        .memType = BufferMemoryType::GPU_STATIC,
        .usage = BufferUsage::SSBO
    };
    gpuBufferId = bufferManager.add(gpuBufferCreateDesc);

    BufferCreateDescription heightmapStagingBufferCreateInfo{
        .size = HeightmapConfig::HEIGHTMAP_SIZE,
        .memType = BufferMemoryType::GPU_STATIC,
        .usage = BufferUsage::SSBO
    };
    heightmapStagingBufferId = bufferManager.add(heightmapStagingBufferCreateInfo);

    ImageCreateDescription desc{};
    desc.arrayLayers = TerrainConfig::INSTANCE_COUNT;
    desc.width = TerrainConfig::RESOLUTION;
    desc.height = TerrainConfig::RESOLUTION;
    desc.format = HeightmapConfig::IMAGE_FORMAT;
    desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    heightmapId = imageSystem.add(desc);
}

void ChunkManager::uploadChunkLinks(BufferManager& bufferManager, VulkanContext& vkCtx) {
    bufferManager.upload(cpuBufferId, chunkLinks.data());
    bufferManager.immediateCopy(vkCtx, cpuBufferId, gpuBufferId, chunkLinksSize);
}

void ChunkManager::updateChunkLinks() {
    uint32_t currentId = 0;

    for (uint32_t x = 0; x < TerrainConfig::SQRT_INSTANCE_COUNT; ++x) {
        for (uint32_t y = 0; y < TerrainConfig::SQRT_INSTANCE_COUNT; ++y) {
            ChunkLink chunk;
            chunk.worldPos = {x, y};
            chunk.heightmapId = currentId;
            chunk.isVisible = 1;

            chunkLinks[currentId] = chunk;
            currentId++;
        }
    }
}

std::array<std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION>, TerrainConfig::INSTANCE_COUNT> ChunkManager::genHeightmap() {
    std::array<std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION>, TerrainConfig::INSTANCE_COUNT> chunkData;
    for (ChunkLink chunk: chunkLinks) {
        chunkData[chunk.heightmapId] = NoiseGenerator::genChunk(chunk.worldPos);
    }
    return chunkData;
}
