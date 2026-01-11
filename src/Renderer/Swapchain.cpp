#include "Swapchain.hpp"

#include <spdlog/spdlog.h>

#include "RHI/Intialization/SwapchainSelector.hpp"

void Swapchain::init(const VulkanContext &vulkanContext, Window &window, uint32_t imageCount) {
    physicalDevice = vulkanContext.physicalDevice;
    surface = vulkanContext.surface;
    device = vulkanContext.device;
    this->imageCount = imageCount;

    SwapchainSelector selector =
        SwapchainSelector::start(vulkanContext.physicalDevice, vulkanContext.surface)
        .addPreferableFormat(VK_FORMAT_B8G8R8A8_SRGB)
        .addPreferableColorSpace(VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        .addPreferableMode(VK_PRESENT_MODE_MAILBOX_KHR)
        .addPreferableMode(VK_PRESENT_MODE_FIFO_KHR)
        .addPreferableMode(VK_PRESENT_MODE_IMMEDIATE_KHR);

    surfaceFormat = selector.pickSurfaceFormat();
    presentMode = selector.pickPresentationMode();
    querySurfaceCapabilities();
    extent = surfaceCapabilities.currentExtent;

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.presentMode = presentMode;
    createInfo.minImageCount = imageCount;
    createInfo.surface = vulkanContext.surface;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    uint32_t queueFamilyIndices[] = {
        vulkanContext.graphicsQueueCtx.index,
        vulkanContext.presentQueueCtx.index
    };
    if (vulkanContext.graphicsQueueCtx.index != vulkanContext.presentQueueCtx.index) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createSwapchain(VK_NULL_HANDLE);
}

void Swapchain::resizeCallback(const uint32_t width, const uint32_t height) {
    spdlog::info("swapchain resize callback -> w:{} h:{}", width, height);
    if (width == 0 || height == 0) return;
    extent.width = width;
    extent.height = height;
    vkDeviceWaitIdle(device);
    createSwapchain(swapchain);
}

void Swapchain::sanitExtent() {
    extent.width = std::clamp(
        extent.width,
        surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width
    );
    extent.height = std::clamp(
        extent.height,
        surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height
    );
}

void Swapchain::querySurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
}

void Swapchain::createSwapchain(VkSwapchainKHR oldSwapchain) {
    querySurfaceCapabilities();
    sanitExtent();
    createInfo.imageExtent = extent;
    createInfo.oldSwapchain = oldSwapchain;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
    getSwapchainImages();
    destroySwapchain(oldSwapchain);
}

void Swapchain::destroySwapchain(VkSwapchainKHR &oldSwapchain) {
    if (swapchain) { vkDestroySwapchainKHR(device, oldSwapchain, nullptr); }
}

void Swapchain::getSwapchainImages() {
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
}

Swapchain::~Swapchain() {
    destroySwapchain(swapchain);
}