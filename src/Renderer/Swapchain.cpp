#include "Swapchain.hpp"

#include "RHI/Intialization/SwapchainSelector.hpp"

void Swapchain::init(const VulkanContext &vulkanContext, Window &window, uint32_t imageCount) {
    this->device = vulkanContext.device;
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
    surfaceCapabilities = selector.getSurfaceCapabilities();
    extent = selector.getExtent(window);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.imageExtent = extent;
    createInfo.presentMode = presentMode;
    createInfo.minImageCount = imageCount;
    createInfo.surface = vulkanContext.surface;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
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

    vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
}

Swapchain::~Swapchain() {
    if (swapchain) { vkDestroySwapchainKHR(device, swapchain, nullptr); }
    for (VkImage image : swapchainImages) {
        if (image) { vkDestroyImage(device, image, nullptr); }
    }
}