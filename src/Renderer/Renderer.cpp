#include "Renderer.hpp"

#include <stdexcept>

#include "Core/Window.hpp"
#include "BarrierBuilder.hpp"
#include "Swapchain.hpp"

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

    swapchain.init(vulkanContext, window);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = vulkanContext.graphicsQueueCtx.index;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = VK_NULL_HANDLE;
    allocInfo.commandBufferCount = 1;

    VkDevice device = vulkanContext.device;
    for (FrameData &frame : frames) {
        vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.imageAvailable);
        vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.inFlight);

        vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &frame.commandPool);
        allocInfo.commandPool = frame.commandPool;
        vkAllocateCommandBuffers(vulkanContext.device, &allocInfo, &frame.commandBuffer);
    }
}

Renderer::~Renderer() {
    for (FrameData &frame : frames) {
        if (frame.imageAvailable) { vkDestroySemaphore(vulkanContext.device, frame.imageAvailable, nullptr); }
        if (frame.inFlight) { vkDestroyFence(vulkanContext.device, frame.inFlight, nullptr); }
        if (frame.commandPool) { vkDestroyCommandPool(vulkanContext.device, frame.commandPool, nullptr); }
    }
}

VkCommandBuffer& Renderer::beginFrame() {
    FrameData& targetFrame = frames[targetFrameIndex];
    VkCommandBuffer& cmd = targetFrame.commandBuffer;

    if (vkWaitForFences(vulkanContext.device, 1, &targetFrame.inFlight, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        throw std::runtime_error("in flight fence waiting failed");
    }

    VkResult result = vkAcquireNextImageKHR(
        vulkanContext.device,
        swapchain.swapchain,
        UINT64_MAX,
        targetFrame.imageAvailable,
        VK_NULL_HANDLE,
        &targetImageViewIndex
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain.recreateSwapchain();
        throw std::runtime_error("out of date khr");
    }

    vkResetFences(vulkanContext.device, 1, &targetFrame.inFlight);

    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    BarrierBuilder::onImage(
        swapchain.scImages[targetImageViewIndex].image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    )
    .access(0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
    .stages(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
    .record(cmd);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapchain.scImages[targetImageViewIndex].imageView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
    colorAttachment.pNext = VK_NULL_HANDLE;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, swapchain.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);

    VkExtent2D extent = swapchain.extent;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return cmd;
}

void Renderer::endFrame() {
    FrameData& targetFrame = frames[targetFrameIndex];
    VkCommandBuffer& cmd = targetFrame.commandBuffer;

    vkCmdEndRendering(cmd);

    BarrierBuilder::onImage(
        swapchain.scImages[targetImageViewIndex].image,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    )
    .access(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0)
    .stages(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
    .record(cmd);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore renderWaitSemaphores[] = { targetFrame.imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = renderWaitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkSemaphore renderSignalSemaphores[] = { swapchain.scImages[targetImageViewIndex].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderSignalSemaphores;

    vkQueueSubmit(vulkanContext.graphicsQueueCtx.queue, 1, &submitInfo, targetFrame.inFlight);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.swapchain;
    presentInfo.pImageIndices = &targetImageViewIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = renderSignalSemaphores;

    vkQueuePresentKHR(vulkanContext.presentQueueCtx.queue, &presentInfo);

    targetFrameIndex = (targetFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}