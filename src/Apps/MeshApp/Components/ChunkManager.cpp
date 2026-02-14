#include "ChunkManager.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <glm/fwd.hpp>

#include "RHI/Image/ImageSystem.hpp"
#include "RHI/Image/ImageCreateDescription.hpp"
#include "Apps/MeshApp/Components/TerrainConfig.hpp"
#include "Apps/MeshApp/Components/NoiseGenerator.hpp"
#include "Apps/MeshApp/Components/HeightmapConfig.hpp"

void ChunkManager::init(glm::vec3* pPlayerPos,  VmaAllocator allocator, ImageSystem& imageSystem, BufferManager& bufferManager) {
    this->pPlayerPos = pPlayerPos;

    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(8);
    noise.SetFrequency(.02);

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

void ChunkManager::diamondUpdateChunkLinks() {
    glm::ivec2 player_coord;
    player_coord.x = this->pPlayerPos->x/TerrainConfig::RESOLUTION;
    player_coord.y = this->pPlayerPos->z/TerrainConfig::RESOLUTION;

    int32_t coords_counter = TerrainConfig::INSTANCE_COUNT - 1;    // The last array position
    // Add the player position as it's the last chunk that should be drawn
    chunkLinks[coords_counter] = {
        .worldPos = player_coord,
        .heightmapId = (uint32_t)coords_counter,
        .isVisible = 1
    };
    coords_counter--;

    glm::ivec2 coord0, coord1, coord2, coord3;
    for (uint32_t i = 0; i < TerrainConfig::DIAMOND_EXPLORATION_RADIUS; i++) {
        int32_t x_pos = player_coord.x + i + 1;
        int32_t x_neg = player_coord.x - i + 1;
        for (uint32_t j = 0; j < (TerrainConfig::DIAMOND_EXPLORATION_RADIUS - i); j++) {
            int32_t y_pos = player_coord.y + j;
            int32_t y_neg = player_coord.y - j;

            // Memory Layout: [Link3][Link2][Link1][Link0]
            ChunkLink* block = &chunkLinks[coords_counter - 3];

            // We write sequentially to the memory block (0, 1, 2, 3).
            block[0] = {
                .worldPos = { x_neg, y_pos },
                .heightmapId = (uint32_t)(coords_counter - 3),
                .isVisible = 1
            };

            // chunkLinks[coords_counter - 2] -> coord2 (+i+1, -j)
            block[1] = {
                .worldPos = { x_pos, y_neg },
                .heightmapId = (uint32_t)(coords_counter - 2),
                .isVisible = 1
            };

            // chunkLinks[coords_counter - 1] -> coord1 (-i+1, -j)
            block[2] = {
                .worldPos = { x_neg, y_neg },
                .heightmapId = (uint32_t)(coords_counter - 1),
                .isVisible = 1
            };

            // chunkLinks[coords_counter - 0] -> coord0 (+i+1, +j)
            block[3] = {
                .worldPos = { x_pos, y_pos },
                .heightmapId = (uint32_t)(coords_counter),
                .isVisible = 1
            };

            coords_counter -= 4;
        }
    }
}

std::array<std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION>, TerrainConfig::INSTANCE_COUNT> ChunkManager::genHeightmap() {
    std::array<std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION>, TerrainConfig::INSTANCE_COUNT> chunkData;
    for (ChunkLink chunk: chunkLinks) {
        chunkData[chunk.heightmapId] = NoiseGenerator::genChunk(noise, chunk.worldPos);
    }
    return chunkData;
}
