#include "ChunkManager.hpp"

#include <array>
#include <cstdint>

#include "RHI/Buffer.hpp"
#include "RHI/Image/ImageSystem.hpp"
#include "RHI/Image/ImageCreateDescription.hpp"
#include "Apps/MeshApp/Components/TerrainConfig.hpp"
#include "Apps/MeshApp/Components/NoiseGenerator.hpp"
#include "Apps/MeshApp/Components/HeightmapConfig.hpp"

void ChunkManager::init(glm::vec3* pPlayerPos,  VmaAllocator allocator, ImageSystem& imageSystem) {
   this->pPlayerPos = pPlayerPos;

    chunkLinks.resize(TerrainConfig::INSTANCE_COUNT);

    cpuBuffer.init(allocator, chunkLinksSize, BufferType::STAGING_UPLOAD);
    gpuBuffer.init(allocator, chunkLinksSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    heightmapStagingBuffer.init(allocator, HeightmapConfig::HEIGHTMAP_SIZE, BufferType::STAGING_UPLOAD);

    ImageCreateDescription desc{};
    desc.arrayLayers = TerrainConfig::INSTANCE_COUNT;
    desc.width = TerrainConfig::RESOLUTION;
    desc.height = TerrainConfig::RESOLUTION;
    desc.format = HeightmapConfig::IMAGE_FORMAT;
    desc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    heightmapId = imageSystem.add(desc);
}

void ChunkManager::uploadChunkLinks(VulkanContext& vkCtx) {
    cpuBuffer.upload(chunkLinks.data(), chunkLinksSize);
    gpuBuffer.immediateCopy(vkCtx, cpuBuffer, chunkLinksSize);
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

void ChunkManager::fillGpuBuffer() {
    std::array<std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION>, TerrainConfig::INSTANCE_COUNT> chunkData;
    for (ChunkLink chunk: chunkLinks) {
        chunkData[chunk.heightmapId] = NoiseGenerator::genChunk(chunk.worldPos);
    }
}

void ChunkManager::uploadHeightmap() {
}
