#include "ChunkManager.hpp"

#include <cstdint>
#include <vector>

void ChunkManager::init(glm::vec3* pPlayerPos,  VmaAllocator allocator) {
   this->pPlayerPos = pPlayerPos;

    chunkLinks.resize(TerrainConfig::INSTANCE_COUNT);

    cpuBuffer.init(allocator, chunkLinksSize, BufferType::STAGING_UPLOAD);
    gpuBuffer.init(allocator, chunkLinksSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
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
