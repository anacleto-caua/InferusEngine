#pragma once

#include <vector>
#include <string>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include "RHITypes.hpp"
#include "Core/Window.hpp"

class VulkanContext {
public:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    QueueContext graphicsQueueCtx;
    QueueContext presentQueueCtx;
    QueueContext transferQueueCtx;
    QueueContext computeQueueCtx;

    VmaAllocator allocator;

private:
    VkDebugUtilsMessengerEXT DEBUG_MESSENGER;

#ifdef NDEBUG
    static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
    static constexpr bool ENABLE_VALIDATION_LAYERS = true;
    std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> VALIDATION_LAYERS_EXTENSION = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#endif


public:
    VulkanContext() = default;
    ~VulkanContext();
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    void init(
        Window &window,
        const std::string &appName,
        const std::string &engineName,
        const std::vector<const char*> &instanceExtensions,
        const std::vector<const char*> &deviceExtensions
    );

    VkCommandBuffer singleTimeCmdBegin(QueueContext ctx);
    void singleTimeCmdSubmit(QueueContext ctx, VkCommandBuffer cmd);

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );

    void setupDebugMessenger();
    void destroyDebugUtilsMessengerEXT();
};
