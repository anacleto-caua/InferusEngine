#include "Renderer.hpp"

#include <stdexcept>

#include "Core/Window.hpp"

void Renderer::init(
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
) {
    vulkanContext.init(
        window,
        appName,
        engineName,
        instanceExtensions,
        deviceExtensions,
        validationLayers,
        validationLayersExts
    );

    swapchain.init(vulkanContext, window, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = vulkanContext.graphicsQueueCtx.index;

    VkDevice &device = vulkanContext.device;
    for (FrameData &frame : frames) {
        if (
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderFinished) != VK_SUCCESS ||
            vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.inFlight) != VK_SUCCESS ||
            vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &frame.commandPool) != VK_SUCCESS
        ) {
            throw std::runtime_error("sync objects creation failed");
        }
    }
}

Renderer::~Renderer() {
    for (FrameData &frame : frames) {
        if (frame.imageAvailable) { vkDestroySemaphore(vulkanContext.device, frame.imageAvailable, nullptr); }
        if (frame.renderFinished) { vkDestroySemaphore(vulkanContext.device, frame.renderFinished, nullptr); }
        if (frame.inFlight) { vkDestroyFence(vulkanContext.device, frame.inFlight, nullptr); }
        if (frame.commandPool) { vkDestroyCommandPool(vulkanContext.device, frame.commandPool, nullptr); }
    }
}
