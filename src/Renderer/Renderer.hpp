#pragma once

#include <array>

#include "RHI/VulkanContext.hpp"
#include "Renderer/Swapchain.hpp"

struct FrameData {
    float deltaTime = 0;
    VkFence inFlight = VK_NULL_HANDLE;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

class Renderer {
public:
    VulkanContext vulkanContext;
    Swapchain swapchain;
    VkPipeline *pipeline;
private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
    uint32_t targetFrameIndex = 0;
    uint32_t targetImageViewIndex = 0;

    VkRect2D scissor{};
    VkViewport viewport{};
    VkRenderingAttachmentInfo colorAttachment{};
    VkRenderingInfo renderingInfo{};

    VkCommandBufferBeginInfo gPipelineCmdBeginInfo{};

    VkSubmitInfo gPipelineCmdSubmitInfo{};
    VkPresentInfoKHR gPipelinePresentInfo{};

public:
    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void init(
        // Vulkan Context
        Window &window,
        const std::string &appName,
        const std::string &engineName,
        const std::vector<const char*> &instanceExtensions,
        const std::vector<const char*> &deviceExtensions,
        const std::vector<const char*> &validationLayers,
        const std::vector<const char*> &validationLayersExts
        // Renderer
        // ...
    );

    void resizeCallback(const uint32_t width, const uint32_t height);

    VkCommandBuffer& beginFrame();
    void endFrame();
    void bindPipeline();
private:
    void createStaticPipelineData();
    void refreshExtent();
};