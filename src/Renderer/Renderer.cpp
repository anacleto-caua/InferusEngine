#include "Renderer.hpp"

#include <stdexcept>

#include "Swapchain.hpp"
#include "Core/Window.hpp"
#include "BarrierBuilder.hpp"

void Renderer::init(
    // Vulkan Context
    Window &window,
    const std::string &appName,
    const std::string &engineName,
    const std::vector<const char*> &instanceExtensions,
    const std::vector<const char*> &deviceExtensions,
    const std::vector<const char*> &validationLayers,
    const std::vector<const char*> &validationLayersExts,
    // Camera
    glm::mat4* pMvp
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
    camera.init(pMvp);
    createStaticPipelineData();
}

Renderer::~Renderer() {
    for (FrameData &frame : frames) {
        if (frame.imageAvailable) { vkDestroySemaphore(vulkanContext.device, frame.imageAvailable, nullptr); }
        if (frame.inFlight) { vkDestroyFence(vulkanContext.device, frame.inFlight, nullptr); }
        if (frame.commandPool) { vkDestroyCommandPool(vulkanContext.device, frame.commandPool, nullptr); }
    }
}

void Renderer::resizeCallback(const uint32_t width, const uint32_t height) {
    if (width == 0 || height == 0) return;
    vkDeviceWaitIdle(vulkanContext.device);
    swapchain.setExtent(width, height);
    swapchain.recreateSwapchain();
    refreshExtent();
}

void Renderer::refreshExtent() {
    VkExtent2D extent = swapchain.extent;
    float f_width = static_cast<float>(extent.width);
    float f_height = static_cast<float>(extent.height);
    float aspect = f_width/f_height;

    viewport.width = f_width;
    viewport.height = f_height;
    scissor.extent = extent;
    renderingInfo.renderArea = { {0, 0}, extent };

    camera.setAspect(aspect);
}

void Renderer::createStaticPipelineData() {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};

    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
    colorAttachment.pNext = VK_NULL_HANDLE;

    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

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

    gPipelineCmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    gPipelineCmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    gPipelineCmdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    gPipelineCmdSubmitInfo.waitSemaphoreCount = 1;
    gPipelineCmdSubmitInfo.pWaitDstStageMask = G_PIPELINE_WAIT_STAGES;
    gPipelineCmdSubmitInfo.commandBufferCount = 1;
    gPipelineCmdSubmitInfo.signalSemaphoreCount = 1;

    gPipelinePresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    gPipelinePresentInfo.swapchainCount = 1;
    gPipelinePresentInfo.pSwapchains = &swapchain.swapchain;
    gPipelinePresentInfo.waitSemaphoreCount = 1;
    gPipelinePresentInfo.pImageIndices = &targetImageViewIndex;

    refreshExtent();
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
    vkBeginCommandBuffer(cmd, &gPipelineCmdBeginInfo);

    BarrierBuilder::onImage(
        swapchain.scImages[targetImageViewIndex].image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    )
    .access(0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
    .stages(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
    .record(cmd);

    colorAttachment.imageView = swapchain.scImages[targetImageViewIndex].imageView;
    vkCmdBeginRendering(cmd, &renderingInfo);

    vkCmdSetViewport(cmd, 0, 1, &viewport);
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

    VkSemaphore renderWaitSemaphores[] = { targetFrame.imageAvailable };
    VkSemaphore renderSignalSemaphores[] = { swapchain.scImages[targetImageViewIndex].renderFinished };

    gPipelineCmdSubmitInfo.pCommandBuffers = &cmd;
    gPipelineCmdSubmitInfo.pWaitSemaphores = renderWaitSemaphores;
    gPipelineCmdSubmitInfo.pSignalSemaphores = renderSignalSemaphores;
    gPipelinePresentInfo.pWaitSemaphores = renderSignalSemaphores;

    vkQueueSubmit(vulkanContext.graphicsQueueCtx.queue, 1, &gPipelineCmdSubmitInfo, targetFrame.inFlight);

    vkQueuePresentKHR(vulkanContext.presentQueueCtx.queue, &gPipelinePresentInfo);

    targetFrameIndex = (targetFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}