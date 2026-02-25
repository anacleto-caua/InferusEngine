#pragma once

#include <array>
#include <cstdint>

#include <glm/fwd.hpp>
#include <glm/ext.hpp>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "Engine/Types.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/InferusRenderer/Image/ImageSystem.hpp"
#include "Engine/InferusRenderer/Buffer/BufferSystem.hpp"
#include "Engine/InferusRenderer/Passes/TerrainRenderer.hpp"

struct QueueContext {
    uint32_t Index;
    VkQueue Queue = VK_NULL_HANDLE;
    VkCommandPool MainCmdPool = VK_NULL_HANDLE;
};

struct FrameData {
    float DeltaTime = 0;
    VkFence InFlight = VK_NULL_HANDLE;
    VkSemaphore ImageAvailable = VK_NULL_HANDLE;
    VkCommandPool CmdPool = VK_NULL_HANDLE;
    VkCommandBuffer CmdBuffer = VK_NULL_HANDLE;
};

struct SwapchainImage {
    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkSemaphore RenderFinished = VK_NULL_HANDLE;
};

class InferusRenderer {
public:
    // Vulkan Context
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    VmaAllocator VmaAllocator;

    QueueContext Graphics;
    QueueContext Present;
    QueueContext Transfer;
    QueueContext Compute;

    BufferSystem BufferSystem;
    static constexpr size_t CREATION_WISE_STAGING_BUFFER_SIZE = 1 * 1024 * 1024;
    ImageSystem ImageSystem;

    // Swapchain
    VkSurfaceKHR Surface;
    VkPresentModeKHR PresentMode {};
    VkSurfaceCapabilitiesKHR SurfaceCapabilities {};
    VkSwapchainCreateInfoKHR SwapchainCreateInfo {};

    VkExtent2D Extent;
    VkSurfaceFormatKHR SurfaceFormat;
    VkSwapchainKHR Swapchain;

    uint32_t SwapchainImageCount = 0;
    VkPresentInfoKHR PresentInfo {};
    std::vector<SwapchainImage> SwapchainImages;

    // Per frame data
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::array<FrameData, MAX_FRAMES_IN_FLIGHT> Frames;
    uint32_t TargetFrameIndex = 0;
    uint32_t TargetImageViewIndex = 0;

    // Drawing -- I imagine this may be shared between all the other pipelines
    static constexpr VkPipelineStageFlags G_PIPELINE_WAIT_STAGES[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkRect2D Scissor {};
    VkViewport Viewport {};
    VkRenderingAttachmentInfo ColorAttachment {};
    VkRenderingInfo RenderingInfo {};

    VkCommandBufferBeginInfo PipelineCmdBeginInfo {};
    VkSubmitInfo PipelineCmdSubmitInfo {};

    TerrainRenderer TerrainRenderer;

public:
    InferusRenderer() = default;
    ~InferusRenderer();
    InferusRenderer(const InferusRenderer&) = delete;
    InferusRenderer& operator=(const InferusRenderer&) = delete;

    InferusResult Init(Window& Window);

    void Render();

    void Resize(uint32_t Width, uint32_t Height);

    VkCommandBuffer SingleTimeCmdBegin(QueueContext& ctx);
    void SingleTimeCmdSubmit(QueueContext& ctx, VkCommandBuffer cmd);

private:
    void RefreshExtent();

    void DestroySwapchain(VkSwapchainKHR OldSwapchain);
    void RecreateSwapchain(VkSwapchainKHR OldSwapchain);
    void CleanupSwapchainImages();

    void QuerySurfaceCapabilities();
private:
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT _DebugMessenger;

    std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> VALIDATION_LAYERS_EXTENSION = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

    static VKAPI_ATTR VkBool32 VKAPI_CALL _DebugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
        VkDebugUtilsMessageTypeFlagsEXT Type,
        const VkDebugUtilsMessengerCallbackDataEXT *CallbackData,
        void *UserData
    );

    void _SetupDebugMessenger();
    void _DestroyDebugUtilsMessengerEXT();
#endif


};
