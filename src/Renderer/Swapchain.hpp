#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Core/Window.hpp"
#include "RHI/VulkanContext.hpp"

struct SCImage {
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
};

class Swapchain {
public:
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkSwapchainKHR swapchain;

    uint32_t imageCount;
    std::vector<SCImage> scImages;

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

    void recreateSwapchain();

    void setExtent(const uint32_t width, const uint32_t height);

private:
    void sanitExtent();
    void querySurfaceCapabilities();

    void createSwapchain(VkSwapchainKHR oldSwapchain);
    void destroySCImages();
    void destroySwapchain(VkSwapchainKHR &oldSwapchain);
};