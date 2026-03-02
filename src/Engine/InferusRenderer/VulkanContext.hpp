#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "Engine/Types.hpp"

struct QueueContext {
    uint32_t Index;
    VkQueue Queue = VK_NULL_HANDLE;
    VkCommandPool MainCmdPool = VK_NULL_HANDLE;
};

namespace VulkanContext {
    // Vulkan Context
    inline VkInstance Instance;
    inline VkPhysicalDevice PhysicalDevice;
    inline VkDevice Device;
    inline VmaAllocator VmaAllocator;

    inline VkSurfaceKHR Surface;
    inline VkSurfaceFormatKHR SurfaceFormat;
    inline VkPresentModeKHR PresentMode;

    inline QueueContext Graphics;
    inline QueueContext Present;
    inline QueueContext Transfer;
    inline QueueContext Compute;
    inline std::vector Queues = { &Graphics, &Present, &Transfer, &Compute };

    InferusResult Create();
    void Destroy();
};

