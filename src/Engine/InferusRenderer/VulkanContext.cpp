#include "VulkanContext.hpp"

#include <array>
#include <vector>

#include "Engine/Core/Window.hpp"

namespace VulkanContext {
    static constexpr std::array<const char*, 5> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            // VMA extensions
            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
        };

    static constexpr std::array<const char*, 1> INSTANCE_EXTENSIONS = {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };

#ifndef NDEBUG
    static constexpr std::array<const char*, 1> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    static constexpr std::array<const char*, 1> VALIDATION_LAYERS_EXTENSION = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#endif

    InferusResult CreateInstance() {
        VkApplicationInfo AppInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Inferus Renderer",
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName = "Inferus Engine",
            .engineVersion = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion = VK_API_VERSION_1_4
        };

        std::vector<const char*> AllInstanceExtensions;
        AllInstanceExtensions.insert(AllInstanceExtensions.end(), INSTANCE_EXTENSIONS.begin(), INSTANCE_EXTENSIONS.end());

        std::vector<const char*> WindowExts = Window::GetRequiredExtensions();
        AllInstanceExtensions.insert(AllInstanceExtensions.end(), WindowExts.begin(), WindowExts.end());

        VkInstanceCreateInfo InstanceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &AppInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(AllInstanceExtensions.size()),
            .ppEnabledExtensionNames = AllInstanceExtensions.data()
        };

        // Validation layers
#ifndef NDEBUG
        AllInstanceExtensions.insert(
                AllInstanceExtensions.end(),
                VALIDATION_LAYERS_EXTENSION.begin(),
                VALIDATION_LAYERS_EXTENSION.end()
                );
        InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        InstanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
#endif

        InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(AllInstanceExtensions.size());
        InstanceCreateInfo.ppEnabledExtensionNames = AllInstanceExtensions.data();

        // Create Vulkan instance
        if (vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance) != VK_SUCCESS) {
            return InferusResult::FAIL;
        }
        return InferusResult::SUCCESS;
    }

    InferusResult PickPhysicalDevice() {
        uint32_t PhysicalDevicesCount;
        vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, nullptr);
        std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
        vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, PhysicalDevices.data());

        int32_t KingOfTheHillScore = -1;
        VkPhysicalDevice KingOfTheHillDevice;
        for (VkPhysicalDevice CurrentPhysicalDevice : PhysicalDevices) {
            int32_t Score = 0;

            // Are required extensions supported
            uint32_t ExtensionCount;
            vkEnumerateDeviceExtensionProperties(
                CurrentPhysicalDevice,
                nullptr,
                &ExtensionCount,
                nullptr
            );
            std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
            vkEnumerateDeviceExtensionProperties(
                CurrentPhysicalDevice,
                nullptr,
                &ExtensionCount,
                AvailableExtensions.data()
            );

            // Are needed features available
            VkPhysicalDeviceFeatures DeviceFeatures;
            vkGetPhysicalDeviceFeatures(CurrentPhysicalDevice, &DeviceFeatures);

            // Apply preference for GPUs and higher resolution
            VkPhysicalDeviceProperties DeviceProperties;
            vkGetPhysicalDeviceProperties(CurrentPhysicalDevice, &DeviceProperties);

            if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                Score += 1000;
            } else if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                Score += 100;
            }

            Score += DeviceProperties.limits.maxImageDimension2D;

            bool ExtensionFound;
            for (const char* Extension : DEVICE_EXTENSIONS) {
                ExtensionFound = false;
                for (const VkExtensionProperties &ExtensionProperties : AvailableExtensions) {
                    if(std::strcmp(Extension, ExtensionProperties.extensionName) == 0) {
                        ExtensionFound = true;
                        break;
                    }
                }

                if(!ExtensionFound) {
                    Score = -1;
                    break;
                }
            }

            // Fight for Koth
            if (Score > KingOfTheHillScore) {
                KingOfTheHillScore = Score;
                KingOfTheHillDevice = CurrentPhysicalDevice;
            }
        }

        if(KingOfTheHillScore < 0) {
            return InferusResult::FAIL;
        }

        // Selecting physical device
        PhysicalDevice = KingOfTheHillDevice;
        return InferusResult::SUCCESS;
    }

    InferusResult PickQueues() {
        struct QueueRequest {
            QueueContext *QueueCtx;
            int32_t LatestScore;
            VkQueueFlags RequiredFlags;
            VkQueueFlags AvoidedFlags;
            bool NeedsPresent;
            bool ScoreUniqueness;
        };

        std::array<QueueRequest, 4> QueueRequests;
        QueueRequests[0] = {
            .QueueCtx = &Graphics,
            .LatestScore = -1,
            .RequiredFlags = VK_QUEUE_GRAPHICS_BIT,
            .AvoidedFlags = 0,
            .NeedsPresent = false,
            .ScoreUniqueness = true
        };
        QueueRequests[1] = {
            .QueueCtx = &Present,
            .LatestScore = -1,
            .RequiredFlags = 0,
            .AvoidedFlags = VK_QUEUE_COMPUTE_BIT,       // Avoid since I don't explicitly support graphics->compute->present yet
            .NeedsPresent = true,
            .ScoreUniqueness = false
        };
        QueueRequests[2] = {
            .QueueCtx = &Transfer,
            .LatestScore = -1,
            .RequiredFlags = VK_QUEUE_TRANSFER_BIT,
            .AvoidedFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
            .NeedsPresent = false,
            .ScoreUniqueness = true
        };
        QueueRequests[3] = {
            .QueueCtx = &Compute,
            .LatestScore = -1,
            .RequiredFlags = VK_QUEUE_COMPUTE_BIT,
            .AvoidedFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
            .NeedsPresent = false,
            .ScoreUniqueness = true
        };

        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> AllQueueFamiliesProperties(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, AllQueueFamiliesProperties.data());

        for (QueueRequest& Req : QueueRequests) {
            constexpr uint32_t SCORE_PER_UNIQUENESS = 1000;
            constexpr uint32_t SCORE_FOR_DESIRED_SUPPORT = 1000;
            constexpr uint32_t SCORE_PER_AVOIDED_FLAG = 100;

            for (uint32_t QueueFamilyIdx = 0; QueueFamilyIdx < QueueFamilyCount; QueueFamilyIdx++) {
                VkQueueFamilyProperties QueueProperties = AllQueueFamiliesProperties[QueueFamilyIdx];

                int32_t Score = 0;
                if (Req.NeedsPresent) {
                    VkBool32 PresentSupport;
                    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, QueueFamilyIdx, Surface, &PresentSupport);
                    if (!PresentSupport) {
                        continue;
                    }
                    Score += SCORE_FOR_DESIRED_SUPPORT;
                }

                if ((QueueProperties.queueFlags & Req.RequiredFlags) != Req.RequiredFlags) {
                    Score = -1;
                    continue;
                }

                if ((QueueProperties.queueFlags & Req.AvoidedFlags) == 0) {
                    Score += SCORE_PER_AVOIDED_FLAG;
                }

                if (Req.ScoreUniqueness) {
                    for (QueueRequest &Req2 : QueueRequests) {
                        if (Req2.LatestScore < 0) { break; }    // Do not compare to not yet picked queues
                        if (QueueFamilyIdx != Req2.QueueCtx->Index) {
                            Score += SCORE_PER_UNIQUENESS;
                        }
                    }
                }

                if (Score > Req.LatestScore) {
                    Req.QueueCtx->Index = QueueFamilyIdx;
                    Req.LatestScore = Score;
                }
            }
        }

        for (QueueRequest Req : QueueRequests) {
            if (Req.LatestScore < 0) {
                return InferusResult::FAIL;
            }
        }

        return InferusResult::SUCCESS;
    }

    InferusResult CreateLogicalDevice() {
        float QueuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {};
        std::vector AllQueues = { &Graphics, &Present, &Transfer, &Compute };
        for (QueueContext* Queue : AllQueues) {
            bool IsUnique = true;
            for (auto CreateInfo : QueueCreateInfos) {
                if (Queue->Index == CreateInfo.queueFamilyIndex) {
                    IsUnique = false;
                    break;
                }
            }
            if (IsUnique) {
                VkDeviceQueueCreateInfo QueueCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = Queue->Index,
                    .queueCount = 1,
                    .pQueuePriorities = &QueuePriority
                };
                QueueCreateInfos.push_back(QueueCreateInfo);
            }
        }

        VkPhysicalDeviceFeatures DeviceFeatures{};
        DeviceFeatures.samplerAnisotropy = VK_TRUE;
        DeviceFeatures.sampleRateShading = VK_TRUE;

        VkPhysicalDeviceFeatures2 DeviceFeatures2{};
        DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        DeviceFeatures2.features = DeviceFeatures;

        VkPhysicalDeviceSynchronization2Features Sync2Features{};
        Sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        Sync2Features.synchronization2 = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeatures DynamicRenderingFeatures{};
        DynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        DynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        DeviceFeatures2.pNext= &Sync2Features;
        Sync2Features.pNext = &DynamicRenderingFeatures;
        DynamicRenderingFeatures.pNext = nullptr;

        VkDeviceCreateInfo DeviceCreateInfo{};
        DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        DeviceCreateInfo.pEnabledFeatures = VK_NULL_HANDLE;
        DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
        DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
        DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        DeviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
        DeviceCreateInfo.pNext = &DeviceFeatures2;

        if (vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device) != VK_SUCCESS) {
            return InferusResult::FAIL;
        }
        return InferusResult::SUCCESS;
    }

    InferusResult CreateVmaAllocator() {
        // Vma Allocator
        VmaAllocatorCreateInfo AllocatorCreateInfo = {};
        AllocatorCreateInfo.physicalDevice = PhysicalDevice;
        AllocatorCreateInfo.device = Device;
        AllocatorCreateInfo.instance = Instance;
        if ( vmaCreateAllocator(&AllocatorCreateInfo, &VmaAllocator) != VK_SUCCESS ) {
            return InferusResult::FAIL;
        }
        return InferusResult::SUCCESS;
    }

    InferusResult Create() {
        CreateInstance();
        PickPhysicalDevice();
        PickQueues();
        CreateLogicalDevice();
        CreateVmaAllocator();
        return InferusResult::SUCCESS;
    }

    void Destroy() {
        if (VmaAllocator) { vmaDestroyAllocator(VmaAllocator); }
        if (Device) { vkDestroyDevice(Device, nullptr); }
        if (Instance) { vkDestroyInstance(Instance, nullptr); }
    }
}
