#include "ShaderStagesBuilder.hpp"

#include <vector>
#include <stdexcept>

#include "Utils/IO.hpp"

ShaderStagesBuilder& ShaderStagesBuilder::addShaderStage(VkShaderStageFlagBits stage, const std::string &filename) {
    ShaderInfo shaderInfo{.stage = stage, .filename = filename};
    shadersInfo.push_back(shaderInfo);
    return *this;
}

void ShaderStagesBuilder::build(VkDevice device, std::vector<VkPipelineShaderStageCreateInfo>& shaderStages) {
    for (ShaderInfo info : shadersInfo) {
        shaderStages.push_back((createShaderStage(device, info)));
    }
}

VkPipelineShaderStageCreateInfo ShaderStagesBuilder::createShaderStage(VkDevice device, ShaderInfo shaderInfo) {
    std::vector<char> shaderCode = IO::binaryRead(shaderInfo.filename);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule{};
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("shader module creation failed for shader: " + shaderInfo.filename);
    }

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = shaderInfo.stage;
    shaderStage.module = shaderModule;
    shaderStage.pName = "main";

    return shaderStage;
}