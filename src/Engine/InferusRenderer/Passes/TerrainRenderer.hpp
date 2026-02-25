#pragma once

#include <glm/fwd.hpp>
#include <glm/ext.hpp>
#include <vulkan/vulkan.h>

class InferusRenderer; // Circular dependency

#include "Engine/Types.hpp"
#include "Engine/InferusRenderer/Image/Image.hpp"
#include "Engine/InferusRenderer/Buffer/Buffer.hpp"
#include "Engine/Systems/Terrain/TerrainConfig.hpp"

struct TerrainDescriptorSet {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
};

struct TerrainPushConstants {
    glm::mat4 CameraMVP;
    glm::vec4 PlayerPosition;
};

class TerrainRenderer {
public:
    // Terrain plane mesh
    BufferId PlaneMeshIndexBufferId;
    VkBuffer PlaneMeshIndexVkBuffer;

    // Terrain pipeline
    VkPipeline TerrainPipeline {};
    VkPipelineLayout TerrainPipelineLayout {};

    // Heightmap
    ImageId HeightmapImageId;
    VkSampler HeightmapTextureSampler;
    BufferId HeightmapStagingBufferId;

    // Chunk to Heightmap linking
    ChunkHeightmapLink ChunkHeightmapLinks[TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT];
    BufferId ChunkHeightmapLinks_CPU;
    BufferId ChunkHeightmapLinks_GPU;

    // Terrain descriptor sets
    TerrainDescriptorSet TerrainDescriptorSet {};

    // Push constants
    TerrainPushConstants TerrainPushConstants {};

public:
    TerrainRenderer() = default;
    ~TerrainRenderer() = default;
    TerrainRenderer(const TerrainRenderer&) = delete;
    TerrainRenderer& operator=(const TerrainRenderer&) = delete;

    InferusResult Init(InferusRenderer &InferusRenderer, BufferId &CreationWiseStagingBufer);
    void FullFeedTerrainData(InferusRenderer &InferusRenderer, ChunkHeightmapLink* ChunkLinkSrc, uint16_t* HeightmapSrc);

    void Destroy(InferusRenderer &InferusRenderer);

    void Render(VkCommandBuffer cmd);

private:

};
