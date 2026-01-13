#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Core/Window.hpp"
#include "RHI/VulkanContext.hpp"

class Swapchain {
public:
private:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;

    VkExtent2D extent;
    uint32_t imageCount;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    VkSwapchainCreateInfoKHR createInfo;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

public:
    Swapchain() = default;
    ~Swapchain();
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    void init(const VulkanContext &vulkanContext, Window &window, uint32_t imageCount);

    void resizeCallback(const uint32_t width, const uint32_t height);

private:
    void sanitExtent();
    void querySurfaceCapabilities();

    void createSwapchain(VkSwapchainKHR oldSwapchain);
    void destroyImageViews();
    void destroySwapchain(VkSwapchainKHR &oldSwapchain);
};