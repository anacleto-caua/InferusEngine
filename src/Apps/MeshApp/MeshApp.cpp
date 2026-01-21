#include "MeshApp.hpp"

#include <cstdint>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "RHI/Buffer.hpp"
#include "TerrainChunkData.hpp"
#include "Apps/MeshApp/AppTypes.hpp"
#include "RHI/Pipeline/ShaderStageBuilder.hpp"
#include "RHI/Pipeline/GraphicsPipelineBuilder.hpp"

void MeshApp::init() {
    const std::string APP_NAME = "MeshApp";
    engine.init(APP_NAME);
    VkDevice device = engine.renderer.vulkanContext.device;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    GraphicsPipelineBuilder builder = GraphicsPipelineBuilder::start()
    .setDefaults()
    .addColorFormat(engine.renderer.swapchain.surfaceFormat.format)
    .addShaderStage(
        ShaderStageBuilder::createShaderStage(
            device,
            VK_SHADER_STAGE_VERTEX_BIT,
            "shaders/terrain.vert.spv"
        )
    )
    .addShaderStage(
        ShaderStageBuilder::createShaderStage(
            device,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            "shaders/terrain.frag.spv"
        )
    );

    pipeline = builder.build(device, pipelineLayout);
    engine.renderer.pipeline = &pipeline;

    for (VkPipelineShaderStageCreateInfo shaderStage : builder.shaderStages) {
        if (shaderStage.module) { vkDestroyShaderModule(device, shaderStage.module, nullptr); }
    }

    createTerrainIndicesBuffer();
}

MeshApp::~MeshApp() {
    VkDevice device = engine.renderer.vulkanContext.device;
    vkDeviceWaitIdle(device);
    if (pipeline) { vkDestroyPipeline(device, pipeline, nullptr); }
    if (pipelineLayout) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
}

void MeshApp::createTerrainIndicesBuffer() {
    std::vector<uint32_t> indices = TerrainChunkData::getIndices();
    uint32_t bufferSize = indices.size() * sizeof(uint32_t);
    terrainIndicesBuffer.init(engine.renderer.vulkanContext.allocator, bufferSize, BufferType::GPU_STATIC, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    terrainIndicesBuffer.immediateUpload(engine.renderer.vulkanContext, indices.data(), bufferSize);
}

void MeshApp::run() {
    while (!shouldClose()) {
        engine.update();
        engine.render([this](VkCommandBuffer cmd) {
            MeshApp::drawCallback(cmd, this);
        });
    }
    vkDeviceWaitIdle(engine.renderer.vulkanContext.device);
}

void MeshApp::drawCallback(VkCommandBuffer commandBuffer, MeshApp* app) {
    if (app->pipeline != VK_NULL_HANDLE) {
        VkExtent2D extent = app->engine.renderer.swapchain.extent;
        float aspect = (float)extent.width / (float)extent.height;
        constexpr glm::float32_t POV = 60.0;
        float focal_length = 1.0f / tan(glm::radians(POV) / 2.0f);
        // Infinite Reverse-Z Projection
        glm::mat4 proj = glm::mat4(0.0f);
        proj[0][0] = focal_length / aspect;

        // -f handles Vulkan Y-flip directly
        proj[1][1] = -focal_length;
        proj[2][2] = 0.0f;    // Z-term is 0 for infinite far plane
        proj[2][3] = -1.0f;   // Standard RH convention
        proj[3][2] = 0.1f;    // zNear

        // VIEW
        // Eye: Up (Y=15) and Back (Z=-20)
        glm::vec3 camPos = glm::vec3(0.0f, 15.0f, -20.0f);
        // Center of grid: (0, 0, 0)
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        // Up: (0, 1, 0)
        glm::mat4 view = glm::lookAt(camPos, target, glm::vec3(0,1,0));
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        PushConstants constants;
        constants.lookAt = mvp;

        vkCmdBindIndexBuffer(commandBuffer, app->terrainIndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdPushConstants(
            commandBuffer,
            app->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstants),
            &constants
        );

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);
        vkCmdDrawIndexed(commandBuffer, TerrainChunkData::INDEX_COUNT, 1, 0, 0, 0);
    }
}

bool MeshApp::shouldClose() {
    return engine.shouldClose();
}