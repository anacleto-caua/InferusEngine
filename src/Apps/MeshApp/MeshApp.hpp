#pragma once

#include <cstdint>

#include "AppTypes.hpp"
#include "RHI/Buffer.hpp"
#include "Core/Engine.hpp"
#include "Components/Heightmap.hpp"
#include "RHI/Pipeline/Pipeline.hpp"
#include "Components/ChunkManager.hpp"
#include "RHI/Pipeline/Descriptor/DescriptorSet.hpp"

class MeshApp {
public:
    PushConstants constants;
private:
    Engine engine;
    Buffer terrainIndicesBuffer;
    Pipeline pipeline;
    Heightmap::HeightmapImage heightmap;
    glm::vec3 playerPos;
    ChunkManager chunkManager;
    static constexpr uint32_t TEXTURE_SAMPLER_BINDING = 0;
    static constexpr uint32_t CHUNK_DATA_BINDING = 1;
    DescriptorSet heightmapDescriptorSet;
public:
    MeshApp() = default;
    ~MeshApp();
    MeshApp(const MeshApp&) = delete;
    MeshApp& operator=(const MeshApp&) = delete;

    void init();

    void run();

    bool shouldClose();

    void drawCallback(VkCommandBuffer commandBuffer);
private:
    void createTerrainIndicesBuffer();
    void createHeightmap();
};
