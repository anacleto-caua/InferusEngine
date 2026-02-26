#include "ImGuiRenderer.hpp"

#include <array>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Engine/InferusRenderer/InferusRenderer.hpp"

InferusResult ImGuiRenderer::Init(Window &Window, InferusRenderer &InferusRenderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // TODO:
    // Check available styles, fonts and setup style

    ImGui_ImplGlfw_InitForVulkan(Window.glfwWindow, true);

    std::array<VkFormat, 1> ColorAttachmentFormats = { InferusRenderer.SurfaceFormat.format };

    VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext = nullptr,
        .viewMask = {},
        .colorAttachmentCount = ColorAttachmentFormats.size(),
        .pColorAttachmentFormats = ColorAttachmentFormats.data(),
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    ImGui_ImplVulkan_PipelineInfo PipelineInfo = {};
    PipelineInfo.PipelineRenderingCreateInfo = PipelineRenderingCreateInfo;

    ImGui_ImplVulkan_InitInfo InitInfo = {};
    InitInfo.ApiVersion = VK_API_VERSION_1_4;
    InitInfo.Instance = InferusRenderer.Instance;
    InitInfo.PhysicalDevice = InferusRenderer.PhysicalDevice;
    InitInfo.Device = InferusRenderer.Device;
    InitInfo.QueueFamily = InferusRenderer.Graphics.Index;
    InitInfo.Queue = InferusRenderer.Graphics.Queue;
    InitInfo.PipelineInfoMain = PipelineInfo;
    // InitInfo.DescriptorPool; -- Leave it alone so the backend creates one with .DescriptorPoolSize
    InitInfo.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    InitInfo.MinImageCount = InferusRenderer.SurfaceCapabilities.minImageCount;
    InitInfo.ImageCount = InferusRenderer.SwapchainImageCount;
    InitInfo.UseDynamicRendering = true;
    InitInfo.MinAllocationSize = 1024 * 1024; // To satisfaz zealous best practices validation layer and waste a little memory.

    ImGui_ImplVulkan_Init(&InitInfo);

    return InferusResult::SUCCESS;
}

void ImGuiRenderer::Destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiRenderer::EarlyRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::LateRender(VkCommandBuffer cmd) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}
