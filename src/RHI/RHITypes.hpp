#pragma once

#include <vulkan/vulkan.h>

struct QueueContext {
    VkQueue queue;
    uint32_t index;
    VkCommandPool mainCmdPool;
};
