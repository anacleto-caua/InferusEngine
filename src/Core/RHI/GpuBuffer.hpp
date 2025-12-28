#pragma once

#include <vulkan/vulkan.h>
#include "./DeviceContext.hpp"
#include "./Types/AppTypes.hpp"
#include "../Resources/Image.hpp"

class GpuBuffer {
public:
    GpuBuffer(DeviceContext& deviceCtx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, QueueContext& queueCtx);
    ~GpuBuffer();

    GpuBuffer(const GpuBuffer&) = delete;
    GpuBuffer& operator=(const GpuBuffer&) = delete;

    void copyFromCpu(const void *sourceData);
    void copyFromCpu(const void *sourceData, size_t size);

    void mapAndWrite(const void* data, VkDeviceSize size);

    void copyFromBuffer(GpuBuffer *srcBuffer);
    void copyFromBuffer(GpuBuffer *srcBuffer, VkDeviceSize size);

    void copyBufferToImage(Image &image);

    QueueContext& m_queueCtx;

    VkBuffer m_vkBuffer;

    VkDeviceSize m_size;

private:
    DeviceContext& m_deviceCtx;
    
    VkDeviceMemory m_memory;
};