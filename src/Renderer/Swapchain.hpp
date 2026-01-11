#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "RHI/VulkanContext.hpp"

class Swapchain {
public:
private:
    VkDevice device;
    VkExtent2D extent;
    uint32_t imageCount;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;

public:
    Swapchain() = default;
    ~Swapchain();

    void init(const VulkanContext &vulkanContext, Window &window, uint32_t imageCount);
private:
};