#include <vulkan/vulkan.h>
#include "Core/RHI/Types/AppTypes.hpp"

struct BarrierConfig {
    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED; 
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED; 
    
    VkImage image = VK_NULL_HANDLE;

    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = 0;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; 
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    
    QueueContext execQueueCtx; 
};

class BarrierBuilder {
public:
    BarrierConfig config;

    static BarrierBuilder transitLayout(QueueContext& execQueueCtx, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
        BarrierBuilder builder;
        builder.config.execQueueCtx = execQueueCtx;
        builder.config.image = image;
        builder.config.oldLayout = oldLayout;
        builder.config.newLayout = newLayout;
        builder.config.srcAccessMask = srcAccessMask;
        builder.config.dstAccessMask = dstAccessMask;
        return builder;
    }

    BarrierBuilder& queues(QueueContext& srcQueueCtx, QueueContext& dstQueueCtx) {
        config.srcQueueFamily = srcQueueCtx.queueFamilyIndex;
        config.dstQueueFamily = dstQueueCtx.queueFamilyIndex;
        return *this;
    }

    BarrierBuilder& stages(VkPipelineStageFlags src, VkPipelineStageFlags dst) {
        config.srcStage = src;
        config.dstStage = dst;
        return *this;
    }
};