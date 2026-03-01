#include "InferusRenderer.hpp"

#include <array>
#include <cstdint>
#include <algorithm>

#include <spdlog/spdlog.h>

#include "Engine/InferusRenderer/Recipes.hpp"
#include "Engine/InferusRenderer/InitSkinner.hpp"

InferusResult InferusRenderer::Init(Window& Window) {
    // Instance
    if ( InitSkinner::Instance::CreateInstance(Window, Instance) != InferusResult::SUCCESS ) {
        spdlog::error("Instance creation failed");
        return InferusResult::FAIL;
    }

#ifndef NDEBUG
    _SetupDebugMessenger();
#endif

    QuerySurfaceCapabilities();
    Extent = SurfaceCapabilities.currentExtent;
    SwapchainImageCount = SurfaceCapabilities.minImageCount + 1;

    SwapchainCreateInfo = {};
    SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCreateInfo.surface = Surface;
    SwapchainCreateInfo.minImageCount = SwapchainImageCount;
    SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainCreateInfo.presentMode = PresentMode;
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t QueueFamilyIndices[] = { Graphics.Index, Present.Index };
    if (Graphics.Index != Present.Index) {
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainCreateInfo.queueFamilyIndexCount = 2;
        SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    } else {
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        SwapchainCreateInfo.queueFamilyIndexCount = 0;
        SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    // Finally create the Swapchain
    Window.GetFramebufferSize(Extent.width, Extent.height);
    RecreateSwapchain(VK_NULL_HANDLE);

    PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pImageIndices = &TargetImageViewIndex;

    // Create per frame info
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo {};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo FenceCreateInfo {};
        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkCommandPoolCreateInfo CommandPoolCreateInfo {};
        CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        CommandPoolCreateInfo.queueFamilyIndex = Graphics.Index;

        VkCommandBufferAllocateInfo AllocInfo {};
        AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        AllocInfo.commandPool = VK_NULL_HANDLE;
        AllocInfo.commandBufferCount = 1;

        for (FrameData &Frame : Frames) {
            vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &Frame.ImageAvailable);
            vkCreateFence(Device, &FenceCreateInfo, nullptr, &Frame.InFlight);

            vkCreateCommandPool(Device, &CommandPoolCreateInfo, nullptr, &Frame.CmdPool);
            AllocInfo.commandPool = Frame.CmdPool;
            vkAllocateCommandBuffers(Device, &AllocInfo, &Frame.CmdBuffer);
        }
    }

    // Fill general rendering information
    Scissor = {
        .offset = { 0, 0 },
        .extent = Extent
    };

    Viewport = {
        .x = 0, .y = 0,
        .width = static_cast<float>(Extent.width),
        .height = static_cast<float>(Extent.height),
        .minDepth = 0.0f, .maxDepth = 1.0f
    };

    ColorAttachment = Recipes::ColorAttachment::Terrain();

    RenderingInfo = {};
    RenderingInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = Extent
    };
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;

    PipelineCmdBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    PipelineCmdSubmitInfo = {};
    PipelineCmdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    PipelineCmdSubmitInfo.waitSemaphoreCount = 1;
    PipelineCmdSubmitInfo.pWaitDstStageMask = G_PIPELINE_WAIT_STAGES;
    PipelineCmdSubmitInfo.commandBufferCount = 1;
    PipelineCmdSubmitInfo.signalSemaphoreCount = 1;

    if (
        ImGuiRenderer.Init(Window, *this) !=  InferusResult::SUCCESS
    ) {
        spdlog::error("Dear ImGui Renderer creation failed");
        return InferusResult::FAIL;
    }

    if (
        TerrainRenderer.Init(*this, CreationWiseStagingBuffer) !=  InferusResult::SUCCESS
    ) {
        spdlog::error("Terrain Renderer creation failed");
        return InferusResult::FAIL;
    }
    return InferusResult::SUCCESS;
}

InferusRenderer::~InferusRenderer() {
    vkDeviceWaitIdle(Device);

    TerrainRenderer.Destroy(*this);
    ImGuiRenderer.Destroy();

    BufferSystem.destroy();
    ImageSystem.destroy();

    for (FrameData &Frame : Frames) {
        if (Frame.ImageAvailable) { vkDestroySemaphore(Device, Frame.ImageAvailable, nullptr); }
        if (Frame.InFlight) { vkDestroyFence(Device, Frame.InFlight, nullptr); }
        if (Frame.CmdPool) { vkDestroyCommandPool(Device, Frame.CmdPool, nullptr); }
    }

    CleanupSwapchainImages();
    DestroySwapchain(Swapchain);

    std::array Queues = { &Graphics, &Present, &Transfer, &Compute };
    for (QueueContext *Queue : Queues) {
        if (Queue->MainCmdPool) { vkDestroyCommandPool(Device, Queue->MainCmdPool, nullptr); }
    }

    if (VmaAllocator) { vmaDestroyAllocator(VmaAllocator); }
    if (Device) { vkDestroyDevice(Device, nullptr); }
    if (Surface) { vkDestroySurfaceKHR(Instance, Surface, nullptr); }

#ifndef NDEBUG
    _DestroyDebugUtilsMessengerEXT();
#endif

    if (Instance) { vkDestroyInstance(Instance, nullptr); }
}

// TODO: Make this async
void InferusRenderer::RecreateSwapchain(VkSwapchainKHR OldSwapchain) {
    vkDeviceWaitIdle(Device);
    SwapchainCreateInfo.imageExtent = Extent;
    SwapchainCreateInfo.oldSwapchain = OldSwapchain;
    SwapchainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;

    if (vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Swapchain creation failed");
    }

    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);
    std::vector<VkImage> ImagesTemp(SwapchainImageCount);
    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, ImagesTemp.data());
    SwapchainImages.resize(SwapchainImageCount);

    CleanupSwapchainImages();
    VkSemaphoreCreateInfo SemaphoreCreateInfo{};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < SwapchainImageCount; i++) {
        SwapchainImages[i].Image = ImagesTemp[i];
        VkImageViewCreateInfo ImageViewCreateInfo =
            Recipes::ImageViewCreateInfo::Swapchain(SwapchainImages[i].Image, SurfaceFormat.format);
        if (
            vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &SwapchainImages[i].ImageView) != VK_SUCCESS ||
            vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &SwapchainImages[i].RenderFinished) != VK_SUCCESS
        ) {
            throw std::runtime_error("Swapchain's Image, ImageView or Semaphore creation failed");
        }
    }
    DestroySwapchain(OldSwapchain);
}

void InferusRenderer::DestroySwapchain(VkSwapchainKHR OldSwapchain) {
    if (Swapchain) { vkDestroySwapchainKHR(Device, OldSwapchain, nullptr); }
}

void InferusRenderer::CleanupSwapchainImages() {
    for (SwapchainImage& SwpchImage : SwapchainImages) {
        if (SwpchImage.ImageView) { vkDestroyImageView(Device, SwpchImage.ImageView, nullptr); }
        if (SwpchImage.RenderFinished) { vkDestroySemaphore(Device, SwpchImage.RenderFinished, nullptr); }
    }
}

void InferusRenderer::QuerySurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities);
}

void InferusRenderer::Resize(uint32_t Width, uint32_t Height) {
    if (Width == 0 || Height == 0) return;
    vkDeviceWaitIdle(Device);
    QuerySurfaceCapabilities();
    VkExtent2D MinExtent = SurfaceCapabilities.minImageExtent;
    VkExtent2D MaxExtent = SurfaceCapabilities.maxImageExtent;
    Extent.width = std::clamp(Width, MinExtent.width, MaxExtent.width);
    Extent.height = std::clamp(Height, MinExtent.height, MaxExtent.height);
    Scissor.extent = Extent;
    Viewport.width = static_cast<float>(Extent.width);
    Viewport.height = static_cast<float>(Extent.height);
    RenderingInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = Extent
    };
    RecreateSwapchain(Swapchain);
}

void InferusRenderer::EarlyRender() {
    ImGuiRenderer.EarlyRender();
}

void InferusRenderer::LateRender() {
    FrameData& TargetFrame = Frames[TargetFrameIndex];
    VkCommandBuffer& cmd = TargetFrame.CmdBuffer;

    vkWaitForFences(Device, 1, &TargetFrame.InFlight, VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        Device,
        Swapchain,
        UINT64_MAX,
        TargetFrame.ImageAvailable,
        VK_NULL_HANDLE,
        &TargetImageViewIndex
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &PipelineCmdBeginInfo);

    vkResetFences(Device, 1, &TargetFrame.InFlight);

    VkImageMemoryBarrier RenderingBarrier =
        Recipes::ImageMemoryBarrier::Rendering::EnableRendering(SwapchainImages[TargetImageViewIndex].Image);
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &RenderingBarrier
    );

    ColorAttachment.imageView = SwapchainImages[TargetImageViewIndex].ImageView;
    vkCmdBeginRendering(cmd, &RenderingInfo);
    vkCmdSetViewport(cmd, 0, 1, &Viewport);
    vkCmdSetScissor(cmd, 0, 1, &Scissor);

    // Actual frame begins

    TerrainRenderer.Render(cmd);

    ImGuiRenderer.LateRender(cmd);

    // Actual frame ends

    vkCmdEndRendering(cmd);

    VkImageMemoryBarrier PresentingBarrier =
        Recipes::ImageMemoryBarrier::Rendering::EnablePresenting(SwapchainImages[TargetImageViewIndex].Image);
    VkPipelineStageFlags srcStage2 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStage2 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage2,
        dstStage2,
        0,
        0, nullptr,
        0, nullptr,
        1, &PresentingBarrier
    );

    vkEndCommandBuffer(cmd);

    VkSemaphore RenderWaitSemaphores[] = { TargetFrame.ImageAvailable };
    VkSemaphore RenderSignalSemaphores[] = { SwapchainImages[TargetImageViewIndex].RenderFinished };

    PipelineCmdSubmitInfo.pCommandBuffers = &cmd;
    PipelineCmdSubmitInfo.pWaitSemaphores = RenderWaitSemaphores;
    PipelineCmdSubmitInfo.pSignalSemaphores = RenderSignalSemaphores;

    PresentInfo.pWaitSemaphores = RenderSignalSemaphores;

    vkQueueSubmit(Graphics.Queue, 1, &PipelineCmdSubmitInfo, TargetFrame.InFlight);
    vkQueuePresentKHR(Present.Queue, &PresentInfo);

    TargetFrameIndex = (TargetFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer InferusRenderer::SingleTimeCmdBegin(QueueContext& ctx) {
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandPool = ctx.MainCmdPool;
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(Device, &AllocInfo, &cmd);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &BeginInfo);

    return cmd;
}

void InferusRenderer::SingleTimeCmdSubmit(QueueContext& ctx, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(ctx.Queue, 1, &SubmitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(ctx.Queue);

    vkFreeCommandBuffers(Device, ctx.MainCmdPool, 1, &cmd);
}

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL InferusRenderer::_DebugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        [[maybe_unused]]void *pUserData)
{
    std::string strMessageType;
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)     strMessageType += "General|";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  strMessageType += "Validation|";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) strMessageType += "Performance|";

    // Result: [Validation] ID: 0x12345 | Message: ...
    std::string msg = fmt::format("[{}] ID: {} | {}",
        strMessageType,
        pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "None",
        pCallbackData->pMessage);

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::debug(msg);
            break;
        default:
            spdlog::critical("Unknown Severity Validation Error: {}", msg);
            break;
    }
    return VK_FALSE;
}

void InferusRenderer::_SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = _DebugMessageCallback;

    PFN_vkCreateDebugUtilsMessengerEXT Func =
        (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(
                Instance,
                "vkCreateDebugUtilsMessengerEXT"
            );
    if ((Func == nullptr) || (Func(Instance, &CreateInfo, nullptr, &_DebugMessenger) != VK_SUCCESS)) {
        throw std::runtime_error("failed to set up debug messenger! it may not be supported by the driver");
    }

}

void InferusRenderer::_DestroyDebugUtilsMessengerEXT() {
    PFN_vkDestroyDebugUtilsMessengerEXT Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (Func != nullptr) {
        Func(Instance, _DebugMessenger, nullptr);
    }
}
#endif
