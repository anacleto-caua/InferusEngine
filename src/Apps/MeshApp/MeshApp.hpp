#pragma once

#include <cstdint>

#include "AppTypes.hpp"
#include "Core/Engine.hpp"
#include "RHI/Buffer/Buffer.hpp"
#include "RHI/Pipeline/Pipeline.hpp"
#include "Components/ChunkManager.hpp"
#include "RHI/Buffer/BufferManager.hpp"
#include "RHI/Pipeline/Descriptor/DescriptorSet.hpp"

class MeshApp {
public:
    PushConstants constants;
private:
    Engine engine;
    BufferId terrainIndicesBufferId;
    Pipeline pipeline;
    ImageSystem imageSystem;
    BufferManager bufferManager;
    ImageId heightmapId;
    glm::vec3 playerPos;
    ChunkManager chunkManager;
    static constexpr uint32_t TEXTURE_SAMPLER_BINDING = 0;
    static constexpr uint32_t CHUNK_DATA_BINDING = 1;
    DescriptorSet heightmapDescriptorSet;
    VkSampler heightmapSampler;
    static const constexpr std::string APP_NAME = "MeshApp";
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
