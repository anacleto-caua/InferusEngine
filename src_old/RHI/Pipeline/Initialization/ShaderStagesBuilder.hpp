#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class ShaderStagesBuilder {
public:
private:
    struct ShaderInfo {
        VkShaderStageFlagBits stage;
        std::string filename;
    };

    std::vector<ShaderInfo> shadersInfo;
public:
    ShaderStagesBuilder() = default;
    ~ShaderStagesBuilder() = default;
    ShaderStagesBuilder(const ShaderStagesBuilder&) = delete;
    ShaderStagesBuilder& operator=(const ShaderStagesBuilder&) = delete;

    ShaderStagesBuilder& addShaderStage(VkShaderStageFlagBits stage, const std::string &filename);
    void build(VkDevice device, std::vector<VkPipelineShaderStageCreateInfo>& shaderStages);

private:
    VkPipelineShaderStageCreateInfo createShaderStage(VkDevice device, ShaderInfo shaderInfo);
};
