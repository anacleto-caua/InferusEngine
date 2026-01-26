#include "ShaderStageBuilder.hpp"

#include <vector>
#include <stdexcept>

#include "Utils/IO.hpp"

namespace ShaderStageBuilder {
    VkPipelineShaderStageCreateInfo createShaderStage(const VkDevice &logicalDevice, VkShaderStageFlagBits stage, const std::string &filename) {
        std::vector<char> shaderCode = IO::binaryRead(filename);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkShaderModule shaderModule{};
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("shader module creation failed for shader: " + filename);
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";

        return shaderStage;
    }
};