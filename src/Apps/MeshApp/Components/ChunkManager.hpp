#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

#include "RHI/Buffer.hpp"
#include "RHI/VulkanContext.hpp"
#include "Apps/MeshApp/Components/TerrainConfig.hpp"

class ChunkManager {
public:
    struct ChunkLink {
        glm::ivec2 worldPos;
        uint32_t heightmapId;
        uint32_t isVisible;
    };

    glm::vec3* pPlayerPos;

    const VkDeviceSize chunkLinksSize = TerrainConfig::INSTANCE_COUNT * sizeof(ChunkLink);

    std::vector<ChunkLink> chunkLinks;

    Buffer cpuBuffer;
    Buffer gpuBuffer;
private:
public:
    ChunkManager() = default;
    ~ChunkManager() = default;
    ChunkManager(const ChunkManager&) = delete;
    ChunkManager& operator=(const ChunkManager&) = delete;

    void init(glm::vec3* pPlayerPos, VmaAllocator allocator);
    void uploadChunkLinks(VulkanContext& vkCtx);
    void updateChunkLinks();
private:
};
