#pragma once

#include <cstdint>

#include "AppTypes.hpp"
#include "RHI/Buffer.hpp"
#include "Core/Engine.hpp"
#include "Components/Heightmap.hpp"
#include "RHI/Descriptor/Descriptor.hpp"

class MeshApp {
public:
    PushConstants constants;
private:
    Engine engine;
    Buffer terrainIndicesBuffer;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    Heightmap::HeightmapImage heightmap;
    static constexpr uint32_t TEXTURE_SAMPLER_BINDING = 0;
    Descriptor heightmapDescriptor;
public:
    MeshApp() = default;
    ~MeshApp();
    MeshApp(const MeshApp&) = delete;
    MeshApp& operator=(const MeshApp&) = delete;

    void init();

    void run();

    bool shouldClose();

    static void drawCallback(VkCommandBuffer commandBuffer, MeshApp* app);
private:
    void createTerrainIndicesBuffer();
    void createHeightmap();
};