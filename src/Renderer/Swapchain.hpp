#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Core/Window.hpp"
#include "RHI/VulkanContext.hpp"

class Swapchain {
public:
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkSwapchainKHR swapchain;

    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
private:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;

    VkPresentModeKHR presentMode;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSwapchainCreateInfoKHR createInfo;
public:
    Swapchain() = default;
    ~Swapchain();
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    void init(const VulkanContext &vulkanContext, Window &window);

    void resizeCallback(const uint32_t width, const uint32_t height);

    void recreateSwapchain();

private:
    void sanitExtent();
    void querySurfaceCapabilities();

    void createSwapchain(VkSwapchainKHR oldSwapchain);
    void destroyImageViews();
    void destroySwapchain(VkSwapchainKHR &oldSwapchain);
};