#include "VulkanContext.hpp"

#include <set>
#include <stdexcept>

#include <vector>
#include <vulkan/vulkan.h>

#include "Intialization/QueueSelector.hpp"
#include "Intialization/DeviceSelector.hpp"
#include "RHI/RHITypes.hpp"

void VulkanContext::init(
    Window &window,
    const std::string &appName,
    const std::string &engineName,
    const std::vector<const char*> &instanceExtensions,
    const std::vector<const char*> &deviceExtensions
) {
    // Instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName.c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 0;

    std::vector<const char*> allInstanceExtensions = instanceExtensions;
    // Validation layers
    if (ENABLE_VALIDATION_LAYERS) {
        // Both parameters are not used anymore but it's recommended to set for backwards compatibility
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        instanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        allInstanceExtensions.insert(allInstanceExtensions.end(), VALIDATION_LAYERS_EXTENSION.begin(), VALIDATION_LAYERS_EXTENSION.end());
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
    }

    instanceCreateInfo.enabledExtensionCount = allInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = allInstanceExtensions.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance");
    }

    if (ENABLE_VALIDATION_LAYERS) {
        setupDebugMessenger();
    }

    // Physical device
    physicalDevice = DeviceSelector::selectPhysicalDevice(instance, deviceExtensions);

    // Surface creation
    window.createSurface(instance, surface);

    // Select queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamiliesProperties.data());

    QueueSelector baseSelector =  QueueSelector::
        startCriteria(nullptr, queueFamiliesProperties)
        .desireExclusivenessAgainst(&graphicsQueueCtx)
        .desireExclusivenessAgainst(&presentQueueCtx)
        .desireExclusivenessAgainst(&transferQueueCtx)
        .desireExclusivenessAgainst(&computeQueueCtx);

    QueueSelector::startCriteria(baseSelector, &presentQueueCtx)
        .requireSurfaceSupport(physicalDevice, surface)
        .clearExclusiveness()
        .select();

    QueueSelector::startCriteria(baseSelector, &graphicsQueueCtx)
        .addRequiredFlags(VK_QUEUE_GRAPHICS_BIT)
        .select();

    QueueSelector::startCriteria(baseSelector, &transferQueueCtx)
        .addRequiredFlags(VK_QUEUE_TRANSFER_BIT)
        .addAvoidedFlags(VK_QUEUE_GRAPHICS_BIT)
        .addAvoidedFlags(VK_QUEUE_COMPUTE_BIT)
        .select();

    QueueSelector::startCriteria(baseSelector, &computeQueueCtx)
        .addRequiredFlags(VK_QUEUE_COMPUTE_BIT)
        .addAvoidedFlags(VK_QUEUE_GRAPHICS_BIT)
        .addAvoidedFlags(VK_QUEUE_TRANSFER_BIT)
        .select();

    spdlog::info("Picked queues:");
    spdlog::info(" - Graphics: {}", graphicsQueueCtx.index);
    spdlog::info(" - Present:  {}", presentQueueCtx.index);
    spdlog::info(" - Transfer: {}", transferQueueCtx.index);
    spdlog::info(" - Compute:  {}", computeQueueCtx.index);

    // Prepare to create queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilyIndexes = {
        graphicsQueueCtx.index,
        presentQueueCtx.index,
        transferQueueCtx.index,
        computeQueueCtx.index
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndexes) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Logical device
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.features = deviceFeatures;

    VkPhysicalDeviceSynchronization2Features sync2Features{};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2Features.synchronization2 = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    deviceFeatures2.pNext= &sync2Features;
    sync2Features.pNext = &dynamicRenderingFeatures;
    dynamicRenderingFeatures.pNext = nullptr;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pEnabledFeatures = VK_NULL_HANDLE;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &deviceFeatures2;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vk device");
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    // Fill the rest of queue context
    std::array queues = {
        &graphicsQueueCtx,
        &presentQueueCtx,
        &transferQueueCtx,
        &computeQueueCtx
    };
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    for (QueueContext *queue : queues) {
        vkGetDeviceQueue(device, queue->index, 0, &queue->queue);
        poolCreateInfo.queueFamilyIndex = queue->index;
        if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &queue->mainCmdPool) != VK_SUCCESS) {
            throw std::runtime_error("main command pool creation failed");
        }
    }
}

VulkanContext::~VulkanContext() {
    std::array queues = {
        &graphicsQueueCtx,
        &presentQueueCtx,
        &transferQueueCtx,
        &computeQueueCtx
    };
    for (QueueContext *queue : queues) {
        if (queue->mainCmdPool) { vkDestroyCommandPool(device, queue->mainCmdPool, nullptr); }
    }
    if (allocator) { vmaDestroyAllocator(allocator); }
    if (device) { vkDestroyDevice(device, nullptr); }
    if (surface) { vkDestroySurfaceKHR(instance, surface, nullptr); }
    if (ENABLE_VALIDATION_LAYERS) { destroyDebugUtilsMessengerEXT(); }
    if (instance) { vkDestroyInstance(instance, nullptr); }
}

VkCommandBuffer VulkanContext::singleTimeCmdBegin(QueueContext queueCtx) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = queueCtx.mainCmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    return cmd;
}

void VulkanContext::singleTimeCmdSubmit(QueueContext ctx, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(ctx.queue, 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(ctx.queue);

    vkFreeCommandBuffers(device, ctx.mainCmdPool, 1, &cmd);
}

void VulkanContext::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(
                instance,
                "vkCreateDebugUtilsMessengerEXT"
            );

    if (
        (func == nullptr) ||
        (func(instance, &createInfo, nullptr, &DEBUG_MESSENGER) != VK_SUCCESS)
    ) {
        throw std::runtime_error("failed to set up debug messenger! it may not be supported by the driver");
    }
}

void VulkanContext::destroyDebugUtilsMessengerEXT() {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(
                instance,
                "vkDestroyDebugUtilsMessengerEXT"
            );

    if (func != nullptr) {
        func(instance, DEBUG_MESSENGER, nullptr);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
) {

    std::string msg = fmt::format("Validation Layer: {}", pCallbackData->pMessage);

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::debug(msg);
            break;
        default:
            spdlog::critical("Unknown Severity Validation Error: {}", msg);
            break;
    }

    return VK_FALSE;
}
