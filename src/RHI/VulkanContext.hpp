#include <iostream>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Core/Window.hpp"
#include "RHITypes.hpp"

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

private:
    const std::vector<char const*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<char const*> VALIDATION_LAYERS_EXTENSION = { "VK_EXT_debug_utils" };
    VkDebugUtilsMessengerEXT DEBUG_MESSENGER;

    #ifdef NDEBUG
        static constexpr bool ENABLE_VALIDATION_LAYERS = false;
    #else
        static constexpr bool ENABLE_VALIDATION_LAYERS = true;
    #endif


public:
    VulkanContext() = default;
    ~VulkanContext();

    void init(Window &window, const std::string &appName, const std::string &engineName, std::vector<const char*> instanceExtensions, std::vector<const char*> deviceExtensions);

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) {

        std::cerr << "Validation Layer: " << pCallbackData->pMessage << '\n';

        return VK_FALSE;
    }

    void setupDebugMessenger();
    void destroyDebugUtilsMessengerEXT();
};