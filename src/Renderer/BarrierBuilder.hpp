#pragma once

#include <vulkan/vulkan.h>

#include "RHI/RHITypes.hpp"

class BarrierBuilder {
public:
    VkImageMemoryBarrier barrier{};
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    static BarrierBuilder onImage(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        BarrierBuilder builder;
        builder.barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        builder.barrier.image = image;
        builder.barrier.oldLayout = oldLayout;
        builder.barrier.newLayout = newLayout;

        // Defaults
        builder.barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        builder.barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        builder.barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        builder.barrier.subresourceRange.baseMipLevel = 0;
        builder.barrier.subresourceRange.levelCount = 1;
        builder.barrier.subresourceRange.baseArrayLayer = 0;
        builder.barrier.subresourceRange.layerCount = 1;

        return builder;
    }

    BarrierBuilder& access(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        return *this;
    }

    BarrierBuilder& queues(const QueueContext &srcQueueCtx, const QueueContext &dstQueueCtx) {
        barrier.srcQueueFamilyIndex = srcQueueCtx.index;
        barrier.dstQueueFamilyIndex = dstQueueCtx.index;
        return *this;
    }

    BarrierBuilder& stages(VkPipelineStageFlags src, VkPipelineStageFlags dst) {
        srcStage = src;
        dstStage = dst;
        return *this;
    }

    BarrierBuilder& baseMipLevel(uint32_t baseMipLevel) {
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        return *this;
    }

    BarrierBuilder& aspectMask(VkImageAspectFlags aspectMask) {
        barrier.subresourceRange.aspectMask = aspectMask;
        return *this;
    }

    BarrierBuilder& levelCount(uint32_t levelCount) {
        barrier.subresourceRange.levelCount = levelCount;
        return *this;
    }

    BarrierBuilder& baseArrayLayer(uint32_t baseArrayLayer) {
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        return *this;
    }

    BarrierBuilder& layerCount(uint32_t layerCount) {
        barrier.subresourceRange.layerCount = layerCount;
        return *this;
    }

    void record(VkCommandBuffer cmd) {
        vkCmdPipelineBarrier(
            cmd,
            srcStage,
            dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }
};