#pragma once

#include <string>

#include <vulkan/vulkan.h>

namespace ShaderStageBuilder {
    VkPipelineShaderStageCreateInfo createShaderStage(const VkDevice &logicalDevice, VkShaderStageFlagBits stage, const std::string& filename);
};