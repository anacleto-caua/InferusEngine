#pragma once

#include "AppTypes.hpp"
#include "RHI/Buffer.hpp"
#include "Core/Engine.hpp"
#include "Components/Heightmap.hpp"

class MeshApp {
public:
    PushConstants constants;
private:
    Engine engine;
    Buffer terrainIndicesBuffer;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    Heightmap::HeightmapImage heightmap;
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