#include "Swapchain.hpp"

#include <cstdint>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <vector>

#include "RHI/Intialization/SwapchainSelector.hpp"

void Swapchain::init(const VulkanContext &vulkanContext, Window &window) {
    physicalDevice = vulkanContext.physicalDevice;
    surface = vulkanContext.surface;
    device = vulkanContext.device;

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
    imageCount = surfaceCapabilities.minImageCount + 1;

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

Swapchain::~Swapchain() {
    destroySCImages();
    destroySwapchain(swapchain);
}

void Swapchain::resizeCallback(const uint32_t width, const uint32_t height) {
    spdlog::info("swapchain resize callback -> w:{} h:{}", width, height);
    if (width == 0 || height == 0) return;
    extent.width = width;
    extent.height = height;
    vkDeviceWaitIdle(device);
    recreateSwapchain();
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

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("swapchain creation failed");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> imagesTemp(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, imagesTemp.data());
    scImages.resize(imageCount);

    destroySCImages();
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < scImages.size(); i++) {
        scImages[i].image = imagesTemp[i];
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = scImages[i].image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = surfaceFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (
            vkCreateImageView(device, &imageViewCreateInfo, nullptr, &scImages[i].imageView) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &scImages[i].renderFinished) != VK_SUCCESS
        ) {
            throw std::runtime_error("SCImage image view or semaphore creation failed");
        }
    }

    destroySwapchain(oldSwapchain);
}

void Swapchain::destroySwapchain(VkSwapchainKHR &oldSwapchain) {
    if (swapchain) { vkDestroySwapchainKHR(device, oldSwapchain, nullptr); }
}

void Swapchain::destroySCImages() {
    for (SCImage scImage : scImages) {
        if (scImage.imageView) { vkDestroyImageView(device, scImage.imageView, nullptr); }
        if (scImage.renderFinished) { vkDestroySemaphore(device, scImage.renderFinished, nullptr); }
    }
}

void Swapchain::recreateSwapchain() {
    createSwapchain(swapchain);
}