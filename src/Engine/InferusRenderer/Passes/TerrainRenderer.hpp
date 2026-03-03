#pragma once

#include <glm/fwd.hpp>
#include <glm/ext.hpp>
#include <vulkan/vulkan.h>

class InferusRenderer; // Circular dependency

#include "Engine/Types.hpp"
#include "Engine/Systems/Terrain/TerrainConfig.hpp"
#include "Engine/Systems/Terrain/TerrainSystem.hpp"
#include "Engine/InferusRenderer/Image/ImageSystem.hpp"
#include "Engine/InferusRenderer/Buffer/BufferSystem.hpp"

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
    BufferSystem::Id PlaneMeshIndexBufferId;
    VkBuffer PlaneMeshIndexVkBuffer;

    // Terrain pipeline
    VkPipeline TerrainPipeline {};
    VkPipelineLayout TerrainPipelineLayout {};

    // Heightmap
    ImageSystem::Id HeightmapImageId;
    VkSampler HeightmapTextureSampler;
    BufferSystem::Id Heightmap_CPU;

    // Chunk to Heightmap linking
    ChunkHeightmapLink ChunkHeightmapLinks[TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT];
    BufferSystem::Id ChunkHeightmapLinks_CPU;
    BufferSystem::Id ChunkHeightmapLinks_GPU;

    // Terrain descriptor sets
    TerrainDescriptorSet TerrainDescriptorSet {};

    // Push constants
    TerrainPushConstants TerrainPushConstants {};

public:
    TerrainRenderer() = default;
    ~TerrainRenderer() = default;
    TerrainRenderer(const TerrainRenderer&) = delete;
    TerrainRenderer& operator=(const TerrainRenderer&) = delete;

    InferusResult Init(BufferSystem::Id &CreationWiseStagingBufer);
    void FullFeedTerrainData(TerrainSystem &TerrainSystem);

    void Destroy();

    void Render(VkCommandBuffer cmd);

private:

};
