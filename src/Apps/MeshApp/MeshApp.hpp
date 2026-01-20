#pragma once

#include "RHI/Buffer.hpp"
#include "Core/Engine.hpp"

class MeshApp {
public:
    Buffer terrainVerticesBuffer;
    Buffer terrainIndicesBuffer;
private:
    Engine engine;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
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
};