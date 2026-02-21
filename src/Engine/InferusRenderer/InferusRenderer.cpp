#include "InferusRenderer.hpp"

#include <array>
#include <cstdint>
#include <algorithm>

#include <spdlog/spdlog.h>

#include "Engine/InferusRenderer/Recipes.hpp"
#include "Engine/Components/Terrain/TerrainConfig.hpp"
#include "Engine/InferusRenderer/ShaderStageBuilder.hpp"
#include "Engine/Components/Terrain/PlaneMeshIndicesGenerator.hpp"

InferusResult InferusRenderer::Init(Window& Window) {
    // Instance
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

    std::vector<const char*> WindowExts = Window.GetRequiredExtensions();
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
        AllInstanceExtensions.insert(AllInstanceExtensions.end(), VALIDATION_LAYERS_EXTENSION.begin(), VALIDATION_LAYERS_EXTENSION.end());
        InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        InstanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
#endif

    InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(AllInstanceExtensions.size());
    InstanceCreateInfo.ppEnabledExtensionNames = AllInstanceExtensions.data();

    // Create Vulkan instance
    if (vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance) != VK_SUCCESS) {
        spdlog::error("Instance creation failed.");
        return InferusResult::FAIL;
    }

#ifndef NDEBUG
    _SetupDebugMessenger();
#endif

    // Physical device
    uint32_t PhysicalDevicesCount;
    vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
    vkEnumeratePhysicalDevices(Instance, &PhysicalDevicesCount, PhysicalDevices.data());

    VkPhysicalDevice KingOfTheHillDevice;
    int32_t KingOfTheHillScore = -1;
    for (VkPhysicalDevice CurrentPhysicalDevice : PhysicalDevices) {
        int32_t Score = 0;

        // Are required extensions supported
        uint32_t ExtensionCount;
        vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, nullptr);
        std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

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
        spdlog::error("No valid Physical Device was found");
        return InferusResult::FAIL;
    }

    // Selecting physical device
    PhysicalDevice = KingOfTheHillDevice;

    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);
    spdlog::info("InferusRenderer's chosen device: {}", DeviceProperties.deviceName);

    // Surface creation
    Window.CreateSurface(Instance, Surface);

    // TODO: Add support for presenting from the compute queue, as in:
    // on graphics: bindings -> draw -[pass to compute]> post processing -> present[still on processing]
    // Sugested by:
    // AMD:     https://gpuopen.com/learn/rdna-performance-guide/#presenting
    // NVDIA:   https://developer.nvidia.com/blog/advanced-api-performance-async-compute-and-overlap/

    // Select queues
    std::array AllQueues = { &Graphics, &Present, &Transfer, &Compute };

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
            spdlog::error("One or more queues couldn't meet their minimum criteria");
            return InferusResult::FAIL;
        }
    }

    spdlog::info("Picked queues:");
    spdlog::info(" - Graphics: {}", Graphics.Index);
    spdlog::info(" - Present: {}", Present.Index);
    spdlog::info(" - Transfer: {}", Transfer.Index);
    spdlog::info(" - Compute: {}", Compute.Index);

    // Creating queue families
    float QueuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos = {};
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

    // Logical device
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
        spdlog::error("Failed to create VkDevice");
        return InferusResult::FAIL;
    }

    // Vma Allocator
    VmaAllocatorCreateInfo AllocatorCreateInfo = {};
    AllocatorCreateInfo.physicalDevice = PhysicalDevice;
    AllocatorCreateInfo.device = Device;
    AllocatorCreateInfo.instance = Instance;

    vmaCreateAllocator(&AllocatorCreateInfo, &VmaAllocator);

    // Memory resources management systems
    BufferSystem.create(VmaAllocator);
    BufferId CreationWiseStagingBuffer;
    {
        BufferCreateDescription CreationWiseStagingBufferCreateDesc = {
            .size = CREATION_WISE_STAGING_BUFFER_SIZE,
            .memType = BufferMemoryType::STAGING_UPLOAD,
            .usage = BufferUsage::STAGING
        };
        CreationWiseStagingBuffer = BufferSystem.add(CreationWiseStagingBufferCreateDesc);
    }
    ImageSystem.create(Device, VmaAllocator);

    // Create the rest of queue context
    VkCommandPoolCreateInfo PoolCreateInfo{};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    for (QueueContext *Queue : AllQueues) {
        vkGetDeviceQueue(Device, Queue->Index, 0, &Queue->Queue);
        PoolCreateInfo.queueFamilyIndex = Queue->Index;
        if (vkCreateCommandPool(Device, &PoolCreateInfo, nullptr, &Queue->MainCmdPool) != VK_SUCCESS) {
            spdlog::error("main command pool creation failed");
            return InferusResult::FAIL;
        }
    }

    // Pick Swapchain format
    bool DesirableSurfacePicked = false;
    VkSurfaceFormatKHR DesirableSurfaceFormat = {
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR
    };

    uint32_t SurfaceFormatCount;
    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr);
    SurfaceFormats.resize(SurfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data());
    for (VkSurfaceFormatKHR CurrSurfaceFormat : SurfaceFormats) {
        if ((CurrSurfaceFormat.format == DesirableSurfaceFormat.format) &&
            (CurrSurfaceFormat.colorSpace == DesirableSurfaceFormat.colorSpace)) {
            SurfaceFormat = CurrSurfaceFormat;
            DesirableSurfacePicked = true;
            break;
        }
    }
    if (!DesirableSurfacePicked) {
        SurfaceFormat = SurfaceFormats[0];
        spdlog::warn("Non desirable surface format picked, image may be damaged");
    }

    // Pick present mode
    std::vector<VkPresentModeKHR> PresentModesTierList = {
        VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR
    };
    uint32_t PresentModesCount;
    std::vector<VkPresentModeKHR> PresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount, nullptr);
    PresentModes.resize(PresentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount, PresentModes.data());
    bool IsPresentModePicked = false;
    for (VkPresentModeKHR ComparingPresentMode : PresentModesTierList ) {
        for (VkPresentModeKHR CurrPresentMode : PresentModes) {
            if (ComparingPresentMode == CurrPresentMode) {
                PresentMode = CurrPresentMode;
                IsPresentModePicked = true;
                break;
            }
        }
        if (IsPresentModePicked) { break; }
    }
    if (!IsPresentModePicked) {
        PresentMode = PresentModes[0];
        spdlog::warn("Non desirable Present Mode picked.");
    }

    QuerySurfaceCapabilities();
    Extent = SurfaceCapabilities.currentExtent;
    SwapchainImageCount = SurfaceCapabilities.minImageCount + 1;

    SwapchainCreateInfo = {};
    SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCreateInfo.surface = Surface;
    SwapchainCreateInfo.minImageCount = SwapchainImageCount;
    SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainCreateInfo.presentMode = PresentMode;
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t QueueFamilyIndices[] = { Graphics.Index, Present.Index };
    if (Graphics.Index != Present.Index) {
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainCreateInfo.queueFamilyIndexCount = 2;
        SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    } else {
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        SwapchainCreateInfo.queueFamilyIndexCount = 0;
        SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    // Finally create the Swapchain
    Window.GetFramebufferSize(Extent.width, Extent.height);
    RecreateSwapchain(VK_NULL_HANDLE);

    PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pImageIndices = &TargetImageViewIndex;

    // Create per frame info
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo {};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo FenceCreateInfo {};
        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkCommandPoolCreateInfo CommandPoolCreateInfo {};
        CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        CommandPoolCreateInfo.queueFamilyIndex = Graphics.Index;

        VkCommandBufferAllocateInfo AllocInfo {};
        AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        AllocInfo.commandPool = VK_NULL_HANDLE;
        AllocInfo.commandBufferCount = 1;

        for (FrameData &Frame : Frames) {
            vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &Frame.ImageAvailable);
            vkCreateFence(Device, &FenceCreateInfo, nullptr, &Frame.InFlight);

            vkCreateCommandPool(Device, &CommandPoolCreateInfo, nullptr, &Frame.CmdPool);
            AllocInfo.commandPool = Frame.CmdPool;
            vkAllocateCommandBuffers(Device, &AllocInfo, &Frame.CmdBuffer);
        }
    }

    // Fill general rendering information
    Scissor = {
        .offset = { 0, 0 },
        .extent = Extent
    };

    Viewport = {
        .x = 0, .y = 0,
        .width = static_cast<float>(Extent.width),
        .height = static_cast<float>(Extent.height),
        .minDepth = 0.0f, .maxDepth = 1.0f
    };

    ColorAttachment = Recipes::ColorAttachment::Terrain();

    RenderingInfo = {};
    RenderingInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = Extent
    };
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;

    PipelineCmdBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    PipelineCmdSubmitInfo = {};
    PipelineCmdSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    PipelineCmdSubmitInfo.waitSemaphoreCount = 1;
    PipelineCmdSubmitInfo.pWaitDstStageMask = G_PIPELINE_WAIT_STAGES;
    PipelineCmdSubmitInfo.commandBufferCount = 1;
    PipelineCmdSubmitInfo.signalSemaphoreCount = 1;

    // Create Terrain Pipeline specific
    {
        VkShaderStageFlags AllStages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

        // TODO: I'm mostly def handling staging buffer in a very very dumb way

        // Terrain heightmap
        {
            ImageCreateDescription HeightmapImageCreateDesc;
            HeightmapImageCreateDesc.width = TerrainConfig::Chunk::RESOLUTION;
            HeightmapImageCreateDesc.height = TerrainConfig::Chunk::RESOLUTION;
            HeightmapImageCreateDesc.arrayLayers = TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT;
            HeightmapImageCreateDesc.format = TerrainConfig::Heightmap::HEIGHTMAP_IMAGE_FORMAT;
            HeightmapImageCreateDesc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            HeightmapImageId = ImageSystem.add(HeightmapImageCreateDesc);

            auto HeightmapSamplerInfo = Recipes::SamplerCreateInfo::HeightmapSampler();
            vkCreateSampler(Device, &HeightmapSamplerInfo, nullptr, &HeightmapTextureSampler);

            BufferCreateDescription HeightmapStagingBufferId_CreateInfo = {
                .size = TerrainConfig::Heightmap::HEIGHTMAP_ALL_IMAGES_SIZE,
                .memType = BufferMemoryType::STAGING_UPLOAD,
                .usage = BufferUsage::STAGING
            };
            HeightmapStagingBufferId = BufferSystem.add(HeightmapStagingBufferId_CreateInfo);
        }

        // Chunk to Heightmap linking
        {
            BufferCreateDescription ChunkHeightmapLinksCPU_CreateDesc = {
                .size = TerrainConfig::ChunkToHeightmapLinking::LINKING_BUFFER_SIZE,
                .memType = BufferMemoryType::STAGING_UPLOAD,
                .usage = BufferUsage::STAGING
            };
            BufferCreateDescription ChunkHeightmapLinksGPU_CreateDesc = {
                .size = TerrainConfig::ChunkToHeightmapLinking::LINKING_BUFFER_SIZE,
                .memType = BufferMemoryType::GPU_STATIC,
                .usage = BufferUsage::SSBO
            };

            ChunkHeightmapLinks_CPU = BufferSystem.add(ChunkHeightmapLinksCPU_CreateDesc);
            ChunkHeightmapLinks_GPU = BufferSystem.add(ChunkHeightmapLinksGPU_CreateDesc);
        }

        // Terrain System Descriptors
        {
            // Heightmap Texture Sampler descriptor
            auto HeightmapImage = ImageSystem.get(HeightmapImageId);
            VkDescriptorImageInfo HeightmapTextureDescriptorImageInfo {};
            HeightmapTextureDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            HeightmapTextureDescriptorImageInfo.imageView = HeightmapImage.imageView;
            HeightmapTextureDescriptorImageInfo.sampler = HeightmapTextureSampler;

            VkDescriptorSetLayoutBinding HeightmapSetLayoutBinding {};
            HeightmapSetLayoutBinding.binding = 0;
            HeightmapSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            HeightmapSetLayoutBinding.descriptorCount = 1;
            HeightmapSetLayoutBinding.stageFlags = AllStages;
            HeightmapSetLayoutBinding.pImmutableSamplers = nullptr;

            VkWriteDescriptorSet HeightmapSamplerWrite {};
            HeightmapSamplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            HeightmapSamplerWrite.dstBinding = 0;
            HeightmapSamplerWrite.dstArrayElement = 0;
            HeightmapSamplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            HeightmapSamplerWrite.descriptorCount = 1;
            HeightmapSamplerWrite.pImageInfo = &HeightmapTextureDescriptorImageInfo;

            // Chunk to Heightmap descriptor
            // TODO: The fact I'm not carrying offsets arround is most definitvelly a bad signal
            auto ChunkLinkBuffer = BufferSystem.get(ChunkHeightmapLinks_GPU);
            VkDescriptorBufferInfo ChunkToHeightmapDescriptorBufferInfo {};
            ChunkToHeightmapDescriptorBufferInfo.buffer = ChunkLinkBuffer.buffer;
            ChunkToHeightmapDescriptorBufferInfo.offset = 0;
            ChunkToHeightmapDescriptorBufferInfo.range = ChunkLinkBuffer.size;

            VkDescriptorSetLayoutBinding ChunkLinkBinding {};
            ChunkLinkBinding.binding = 1;
            ChunkLinkBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            ChunkLinkBinding.descriptorCount = 1;
            ChunkLinkBinding.stageFlags = AllStages;
            ChunkLinkBinding.pImmutableSamplers = nullptr;

            VkWriteDescriptorSet ChunkLinkSSBOWrite {};
            ChunkLinkSSBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            ChunkLinkSSBOWrite.dstBinding = 1;
            ChunkLinkSSBOWrite.dstArrayElement = 0;
            ChunkLinkSSBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            ChunkLinkSSBOWrite.descriptorCount = 1;
            ChunkLinkSSBOWrite.pBufferInfo = &ChunkToHeightmapDescriptorBufferInfo;

            // Descriptor layout
            std::array<VkDescriptorSetLayoutBinding, 2> LayoutBindings = {
                HeightmapSetLayoutBinding,
                ChunkLinkBinding
            };
            VkDescriptorSetLayoutCreateInfo TerrainDescriptorSetLayoutCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<uint32_t>(LayoutBindings.size()),
                .pBindings = LayoutBindings.data()
            };
            if (
                vkCreateDescriptorSetLayout(
                    Device,
                    &TerrainDescriptorSetLayoutCreateInfo,
                    nullptr,
                    &TerrainDescriptorSet.layout
                ) != VK_SUCCESS
                )
            {
                spdlog::error("Terrain descriptor set layout creation failed");
                return InferusResult::FAIL;
            }

            // Descriptor pool
            VkDescriptorPoolSize SamplerHeightmapPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1
            };
            VkDescriptorPoolSize SSBOHeightmapPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1
            };
            std::array<VkDescriptorPoolSize, 2> PoolSize = {
                SamplerHeightmapPoolSize,
                SSBOHeightmapPoolSize
            };

            VkDescriptorPoolCreateInfo PoolInfo{};
            PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSize.size());
            PoolInfo.pPoolSizes = PoolSize.data();
            PoolInfo.maxSets = 1;

            if (vkCreateDescriptorPool(Device, &PoolInfo, nullptr, &TerrainDescriptorSet.pool) != VK_SUCCESS) {
                spdlog::error("Descriptor pool creation failed");
                return InferusResult::FAIL;
            }

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = TerrainDescriptorSet.pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &TerrainDescriptorSet.layout;

            if (vkAllocateDescriptorSets(Device, &allocInfo, &TerrainDescriptorSet.set) != VK_SUCCESS) {
                spdlog::error("Descriptor set allocation failed");
                return InferusResult::FAIL;
            }

            std::array<VkWriteDescriptorSet, 2> TerrainWrites = {
                HeightmapSamplerWrite, ChunkLinkSSBOWrite
            };
            for (VkWriteDescriptorSet& write : TerrainWrites) {
                write.dstSet = TerrainDescriptorSet.set;
            }

            vkUpdateDescriptorSets(Device, static_cast<uint32_t>(TerrainWrites.size()), TerrainWrites.data(), 0, nullptr);
        }

        TerrainPipelineLayout = {};
        VkPushConstantRange TerrainPushConstantRange = {
            .stageFlags = AllStages,
            .offset = 0,
            .size = static_cast<uint32_t>(sizeof(TerrainPushConstants))
        };

        VkPipelineLayoutCreateInfo TerrainPipelineLayoutCreateInfo {};
        TerrainPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        TerrainPipelineLayoutCreateInfo.setLayoutCount = 1;
        TerrainPipelineLayoutCreateInfo.pSetLayouts = &TerrainDescriptorSet.layout;
        TerrainPipelineLayoutCreateInfo.pPushConstantRanges = &TerrainPushConstantRange;
        TerrainPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        if (vkCreatePipelineLayout(Device, &TerrainPipelineLayoutCreateInfo, nullptr, &TerrainPipelineLayout) != VK_SUCCESS) {
            spdlog::error("Terrain pipeline layout creation failed");
            return InferusResult::FAIL;
        }

        // Finally creating the terrain VkPipeline itself
        // TODO: Check if it's needed since we're already using dynamic rendering
        std::vector<VkDynamicState> DynamicStates {};
        DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo DynamicState {};
        DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        DynamicState.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
        DynamicState.pDynamicStates = DynamicStates.data();

        // TODO: Do I need a depth attachment?
        // Can I use this instead of picking chunks in order? Compare performance
        VkFormat DepthAttachmentFormat = VK_FORMAT_UNDEFINED;

        std::array<VkFormat, 1> ColorAttachmentFormats = { SurfaceFormat.format };
        auto TerrainColorBlendState = Recipes::Pipeline::Parts::ColorBlendAttachmentState::Default();
        std::vector<VkPipelineColorBlendAttachmentState> TerrainBlendAttachments(ColorAttachmentFormats.size(), TerrainColorBlendState);
        VkPipelineRenderingCreateInfo RenderingCreateInfo{};
        RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        RenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentFormats.size());
        RenderingCreateInfo.pColorAttachmentFormats = ColorAttachmentFormats.data();
        RenderingCreateInfo.depthAttachmentFormat = DepthAttachmentFormat;
        RenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        auto VertexInput = Recipes::Pipeline::Parts::VertexInput::Default();
        auto InputAssembly = Recipes::Pipeline::Parts::InputAssembly::Default();
        auto ViewportState = Recipes::Pipeline::Parts::ViewportState::Default();
        auto Rasterization = Recipes::Pipeline::Parts::Rasterization::Default();
        auto Multisample = Recipes::Pipeline::Parts::Multisample::Default();
        auto DepthStencil = Recipes::Pipeline::Parts::DepthStencil::Default();
        auto ColorBlendState = Recipes::Pipeline::Parts::ColorBlendState::Default(TerrainBlendAttachments);

        VkGraphicsPipelineCreateInfo TerrainPipelineCreateInfo {};
        TerrainPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        TerrainPipelineCreateInfo.pVertexInputState = &VertexInput;
        TerrainPipelineCreateInfo.pInputAssemblyState = &InputAssembly;
        TerrainPipelineCreateInfo.pViewportState = &ViewportState;
        TerrainPipelineCreateInfo.pRasterizationState = &Rasterization;
        TerrainPipelineCreateInfo.pMultisampleState = &Multisample;
        TerrainPipelineCreateInfo.pDepthStencilState = &DepthStencil;
        TerrainPipelineCreateInfo.pColorBlendState = &ColorBlendState;
        TerrainPipelineCreateInfo.pDynamicState = &DynamicState;
        TerrainPipelineCreateInfo.layout = TerrainPipelineLayout;
        TerrainPipelineCreateInfo.basePipelineIndex = -1;
        TerrainPipelineCreateInfo.pNext = &RenderingCreateInfo;

        // Add shaders
        std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
        std::vector<char> ShaderBuffer;
        ShaderBuffer.reserve(4096);

        ShaderStages.push_back(
            ShaderBuilder::CreateShaderStage(
                VK_SHADER_STAGE_VERTEX_BIT,
                "shaders/terrain.vert.spv",
                ShaderBuffer,
                Device
            )
        );
        ShaderStages.push_back(
            ShaderBuilder::CreateShaderStage(
                VK_SHADER_STAGE_FRAGMENT_BIT,
                "shaders/terrain.frag.spv",
                ShaderBuffer,
                Device
            )
        );

        TerrainPipelineCreateInfo.stageCount = static_cast<uint32_t>(ShaderStages.size());
        TerrainPipelineCreateInfo.pStages = ShaderStages.data();

        if (
            vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &TerrainPipelineCreateInfo, nullptr, &TerrainPipeline) != VK_SUCCESS
            ) {
            spdlog::error("Terrain Pipeline creation failed.");
            return InferusResult::FAIL;
        }

        // TODO: Add caching
        // As of now just destroy the shader modules
        for (auto ShaderStage : ShaderStages) {
            if (ShaderStage.module) { vkDestroyShaderModule(Device, ShaderStage.module, nullptr); }
        }
    }

    // --- Creation wise command buffer begins
    VkCommandBuffer CreationWiseTransferCmdBuffer = SingleTimeCmdBegin(Transfer);

    // Terrain plane mesh indices buffer
    std::array<uint32_t, TerrainConfig::Chunk::INDICES_COUNT> TerrainPlaneMeshIndices;
    PlaneMeshIndicesGenerator::GetIndices(TerrainPlaneMeshIndices.data());

    BufferCreateDescription Terrain_PlaneMeshIndexBufferCreateDescription = {
        .size = TerrainConfig::Chunk::INDICES_BUFFER_SIZE,
        .memType = BufferMemoryType::GPU_STATIC,
        .usage = BufferUsage::INDEX,
    };
    Terrain_PlaneMeshIndexBufferId = BufferSystem.add(Terrain_PlaneMeshIndexBufferCreateDescription);

    BufferSystem.upload(CreationWiseTransferCmdBuffer, CreationWiseStagingBuffer, Terrain_PlaneMeshIndexBufferId, TerrainPlaneMeshIndices.data(), TerrainConfig::Chunk::INDICES_BUFFER_SIZE);

    // --- Creation wise command buffer ends
    SingleTimeCmdSubmit(Transfer, CreationWiseTransferCmdBuffer);

    // Zeroing terrain push constants
    TerrainPushConstants = {
        .CameraMVP = glm::mat4(0),
        .PlayerPosition = glm::vec4(0)
    };

    BufferSystem.del(CreationWiseStagingBuffer);
    return InferusResult::SUCCESS;
}

InferusRenderer::~InferusRenderer() {
    vkDeviceWaitIdle(Device);

    BufferSystem.del(Terrain_PlaneMeshIndexBufferId);
    BufferSystem.destroy();
    ImageSystem.destroy();

    if (HeightmapTextureSampler) { vkDestroySampler(Device, HeightmapTextureSampler, nullptr); }

    if (TerrainDescriptorSet.pool) { vkDestroyDescriptorPool(Device, TerrainDescriptorSet.pool, nullptr); }
    if (TerrainDescriptorSet.layout) { vkDestroyDescriptorSetLayout(Device, TerrainDescriptorSet.layout, nullptr); }

    if (TerrainPipeline) { vkDestroyPipeline(Device, TerrainPipeline, nullptr); }
    if (TerrainPipelineLayout) { vkDestroyPipelineLayout(Device, TerrainPipelineLayout, nullptr); }

    for (FrameData &Frame : Frames) {
        if (Frame.ImageAvailable) { vkDestroySemaphore(Device, Frame.ImageAvailable, nullptr); }
        if (Frame.InFlight) { vkDestroyFence(Device, Frame.InFlight, nullptr); }
        if (Frame.CmdPool) { vkDestroyCommandPool(Device, Frame.CmdPool, nullptr); }
    }

    CleanupSwapchainImages();
    DestroySwapchain(Swapchain);

    std::array Queues = { &Graphics, &Present, &Transfer, &Compute };
    for (QueueContext *Queue : Queues) {
        if (Queue->MainCmdPool) { vkDestroyCommandPool(Device, Queue->MainCmdPool, nullptr); }
    }

    if (VmaAllocator) { vmaDestroyAllocator(VmaAllocator); }
    if (Device) { vkDestroyDevice(Device, nullptr); }
    if (Surface) { vkDestroySurfaceKHR(Instance, Surface, nullptr); }

#ifndef NDEBUG
    _DestroyDebugUtilsMessengerEXT();
#endif

    if (Instance) { vkDestroyInstance(Instance, nullptr); }
}

// TODO: Make this async
void InferusRenderer::RecreateSwapchain(VkSwapchainKHR OldSwapchain) {
    vkDeviceWaitIdle(Device);
    SwapchainCreateInfo.imageExtent = Extent;
    SwapchainCreateInfo.oldSwapchain = OldSwapchain;
    SwapchainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;

    if (vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Swapchain creation failed");
    }

    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);
    std::vector<VkImage> ImagesTemp(SwapchainImageCount);
    vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, ImagesTemp.data());
    SwapchainImages.resize(SwapchainImageCount);

    CleanupSwapchainImages();
    VkSemaphoreCreateInfo SemaphoreCreateInfo{};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < SwapchainImageCount; i++) {
        SwapchainImages[i].Image = ImagesTemp[i];
        VkImageViewCreateInfo ImageViewCreateInfo =
            Recipes::ImageViewCreateInfo::Swapchain(SwapchainImages[i].Image, SurfaceFormat.format);
        if (
            vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &SwapchainImages[i].ImageView) != VK_SUCCESS ||
            vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &SwapchainImages[i].RenderFinished) != VK_SUCCESS
        ) {
            throw std::runtime_error("Swapchain's Image, ImageView or Semaphore creation failed");
        }
    }
    DestroySwapchain(OldSwapchain);
}

void InferusRenderer::DestroySwapchain(VkSwapchainKHR OldSwapchain) {
    if (Swapchain) { vkDestroySwapchainKHR(Device, OldSwapchain, nullptr); }
}

void InferusRenderer::CleanupSwapchainImages() {
    for (SwapchainImage& SwpchImage : SwapchainImages) {
        if (SwpchImage.ImageView) { vkDestroyImageView(Device, SwpchImage.ImageView, nullptr); }
        if (SwpchImage.RenderFinished) { vkDestroySemaphore(Device, SwpchImage.RenderFinished, nullptr); }
    }
}

void InferusRenderer::QuerySurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities);
}

void InferusRenderer::Resize(uint32_t Width, uint32_t Height) {
    if (Width == 0 || Height == 0) return;
    vkDeviceWaitIdle(Device);
    QuerySurfaceCapabilities();
    VkExtent2D MinExtent = SurfaceCapabilities.minImageExtent;
    VkExtent2D MaxExtent = SurfaceCapabilities.maxImageExtent;
    Extent.width = std::clamp(Width, MinExtent.width, MaxExtent.width);
    Extent.height = std::clamp(Height, MinExtent.height, MaxExtent.height);
    Scissor.extent = Extent;
    Viewport.width = static_cast<float>(Extent.width);
    Viewport.height = static_cast<float>(Extent.height);
    RenderingInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = Extent
    };
    RecreateSwapchain(Swapchain);
}

void InferusRenderer::FullFeedTerrainData(ChunkHeightmapLink* ChunkLinkSrc, uint16_t* HeightmapSrc) {
    VkCommandBuffer cmd = SingleTimeCmdBegin(Transfer);

    // Copy chunk link buffer
    BufferSystem.upload(cmd, ChunkHeightmapLinks_CPU, ChunkHeightmapLinks_GPU, ChunkLinkSrc, TerrainConfig::ChunkToHeightmapLinking::LINKING_BUFFER_SIZE);

    // Upload heightmap to staging buffer
    BufferSystem.upload(HeightmapStagingBufferId, HeightmapSrc, TerrainConfig::Heightmap::HEIGHTMAP_ALL_IMAGES_SIZE);
    Buffer HeightmapStagingBuffer = BufferSystem.get(HeightmapStagingBufferId);
    Image HeightmapImage = ImageSystem.get(HeightmapImageId);

    // Transfer data to heightmap image
    VkImageMemoryBarrier barrier1 = Recipes::ImageMemoryBarrier::TransferDest(HeightmapImage);
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier1
    );

    VkBufferImageCopy HeightmapCopy = Recipes::BufferImageCopy::Default(HeightmapImage);
    vkCmdCopyBufferToImage(
        cmd,
        HeightmapStagingBuffer.buffer,
        HeightmapImage.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &HeightmapCopy
    );

    VkImageMemoryBarrier barrier2 = Recipes::ImageMemoryBarrier::ShaderRead(HeightmapImage);
    barrier2.srcQueueFamilyIndex = Transfer.Index;
    barrier2.dstQueueFamilyIndex = Graphics.Index;
    barrier2.dstAccessMask = 0;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier2
    );

    SingleTimeCmdSubmit(Transfer, cmd);

    cmd = SingleTimeCmdBegin(Graphics);

    VkImageMemoryBarrier barrier3 = Recipes::ImageMemoryBarrier::ShaderRead(HeightmapImage);
    barrier3.srcQueueFamilyIndex = Transfer.Index;
    barrier3.dstQueueFamilyIndex = Graphics.Index;
    barrier3.srcAccessMask = 0;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier3
    );

    SingleTimeCmdSubmit(Graphics, cmd);
}

void InferusRenderer::Render() {
    FrameData& TargetFrame = Frames[TargetFrameIndex];
    VkCommandBuffer& cmd = TargetFrame.CmdBuffer;

    vkWaitForFences(Device, 1, &TargetFrame.InFlight, VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        Device,
        Swapchain,
        UINT64_MAX,
        TargetFrame.ImageAvailable,
        VK_NULL_HANDLE,
        &TargetImageViewIndex
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &PipelineCmdBeginInfo);

    vkResetFences(Device, 1, &TargetFrame.InFlight);

    VkImageMemoryBarrier RenderingBarrier =
        Recipes::ImageMemoryBarrier::Rendering::EnableRendering(SwapchainImages[TargetImageViewIndex].Image);
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &RenderingBarrier
    );

    ColorAttachment.imageView = SwapchainImages[TargetImageViewIndex].ImageView;
    vkCmdBeginRendering(cmd, &RenderingInfo);
    vkCmdSetViewport(cmd, 0, 1, &Viewport);
    vkCmdSetScissor(cmd, 0, 1, &Scissor);

    // Actual frame begins

    vkCmdBindIndexBuffer(cmd, BufferSystem.get(Terrain_PlaneMeshIndexBufferId).buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(
        cmd,
        TerrainPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(TerrainPushConstants),
        &TerrainPushConstants
    );

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, TerrainPipeline);
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        TerrainPipelineLayout,
        0, // Probably a bad idea the way I carry this binding value lol TEXTURE_SAMPLER_BINDING,
        1,
        &TerrainDescriptorSet.set,
        0,
        nullptr
    );

    vkCmdDrawIndexed(cmd, TerrainConfig::Chunk::INDICES_COUNT, TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT, 0, 0, 0);

    // Actual frame ends

    vkCmdEndRendering(cmd);

    VkImageMemoryBarrier PresentingBarrier =
        Recipes::ImageMemoryBarrier::Rendering::EnablePresenting(SwapchainImages[TargetImageViewIndex].Image);
    VkPipelineStageFlags srcStage2 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStage2 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage2,
        dstStage2,
        0,
        0, nullptr,
        0, nullptr,
        1, &PresentingBarrier
    );

    vkEndCommandBuffer(cmd);

    VkSemaphore RenderWaitSemaphores[] = { TargetFrame.ImageAvailable };
    VkSemaphore RenderSignalSemaphores[] = { SwapchainImages[TargetImageViewIndex].RenderFinished };

    PipelineCmdSubmitInfo.pCommandBuffers = &cmd;
    PipelineCmdSubmitInfo.pWaitSemaphores = RenderWaitSemaphores;
    PipelineCmdSubmitInfo.pSignalSemaphores = RenderSignalSemaphores;

    PresentInfo.pWaitSemaphores = RenderSignalSemaphores;

    vkQueueSubmit(Graphics.Queue, 1, &PipelineCmdSubmitInfo, TargetFrame.InFlight);
    vkQueuePresentKHR(Present.Queue, &PresentInfo);

    TargetFrameIndex = (TargetFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer InferusRenderer::SingleTimeCmdBegin(QueueContext& ctx) {
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandPool = ctx.MainCmdPool;
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(Device, &AllocInfo, &cmd);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &BeginInfo);

    return cmd;
}

void InferusRenderer::SingleTimeCmdSubmit(QueueContext& ctx, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(ctx.Queue, 1, &SubmitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(ctx.Queue);

    vkFreeCommandBuffers(Device, ctx.MainCmdPool, 1, &cmd);
}

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL InferusRenderer::_DebugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        [[maybe_unused]]void *pUserData)
{
    std::string strMessageType;
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)     strMessageType += "General|";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  strMessageType += "Validation|";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) strMessageType += "Performance|";

    // Result: [Validation] ID: 0x12345 | Message: ...
    std::string msg = fmt::format("[{}] ID: {} | {}",
        strMessageType,
        pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "None",
        pCallbackData->pMessage);

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

void InferusRenderer::_SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = _DebugMessageCallback;

    PFN_vkCreateDebugUtilsMessengerEXT Func =
        (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(
                Instance,
                "vkCreateDebugUtilsMessengerEXT"
            );
    if ((Func == nullptr) || (Func(Instance, &CreateInfo, nullptr, &_DebugMessenger) != VK_SUCCESS)) {
        throw std::runtime_error("failed to set up debug messenger! it may not be supported by the driver");
    }

}

void InferusRenderer::_DestroyDebugUtilsMessengerEXT() {
    PFN_vkDestroyDebugUtilsMessengerEXT Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (Func != nullptr) {
        Func(Instance, _DebugMessenger, nullptr);
    }
}
#endif
