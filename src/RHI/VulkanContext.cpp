#include "VulkanContext.hpp"

#include <set>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Intialization/DeviceSelector.hpp"
#include "Intialization/QueueSelector.hpp"

void VulkanContext::init(Window &window, const std::string &appName, const std::string &engineName, std::vector<const char*> instanceExtensions, std::vector<const char*> deviceExtensions) {
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

    // Validation layers
    if (ENABLE_VALIDATION_LAYERS) {
        // Both parameters are not used anymore but it's recommended to set for backwards compatibility
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        instanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        instanceExtensions.insert(instanceExtensions.end(), VALIDATION_LAYERS_EXTENSION.begin(), VALIDATION_LAYERS_EXTENSION.end());
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
    }

    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

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

    // Create queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilyIndexes = {
        graphicsQueueCtx.index,
        presentQueueCtx.index,
        transferQueueCtx.index,
        computeQueueCtx.index
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndexes) {
        if (queueFamilyIndex < 0) {
            throw std::runtime_error("no valid queue found!");
        }

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    std::cout << "Picked queues -v-\n";
    std::cout << "Graphics queue index: " << graphicsQueueCtx.index << "\n";
    std::cout << "Present queue index: " << presentQueueCtx.index << "\n";
    std::cout << "Transfer queue index: " << transferQueueCtx.index << "\n";
    std::cout << "Compute queue index: " << computeQueueCtx.index << "\n";

    // Logical device
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkPhysicalDeviceSynchronization2Features sync2Features = {};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2Features.synchronization2 = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &sync2Features;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vk device");
    }
}

VulkanContext::~VulkanContext() {
    if (device) { vkDestroyDevice(device, nullptr); }
    if (ENABLE_VALIDATION_LAYERS) { destroyDebugUtilsMessengerEXT(); }
    if (instance) { vkDestroyInstance(instance, nullptr); }
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
