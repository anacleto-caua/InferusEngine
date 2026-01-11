#pragma once

#include "RHI/VulkanContext.hpp"
#include "Renderer/Swapchain.hpp"

struct FrameData {
    float deltaTime;
    // ...
};

class Renderer {
public:
    VulkanContext vulkanContext;
    Swapchain swapchain;
private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    std::array<FrameData, MAX_FRAMES_IN_FLIGHT> frames;
    uint32_t currentFrame = 0;

public:
    Renderer() = default;
    ~Renderer();

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
private:
};