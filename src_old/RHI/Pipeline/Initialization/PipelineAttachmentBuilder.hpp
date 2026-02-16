#pragma once

#include <vulkan/vulkan.h>

struct AttachmentBuild {
    VkAttachmentDescription description{};
    VkAttachmentReference reference{};
};

class PipelineAttachmentBuilder {
public:
    AttachmentBuild attachment;

    PipelineAttachmentBuilder(uint32_t attachmentId, VkImageLayout imageRefLayout, VkFormat format);
    ~PipelineAttachmentBuilder() = default;
    PipelineAttachmentBuilder(const PipelineAttachmentBuilder&) = delete;
    PipelineAttachmentBuilder& operator=(const PipelineAttachmentBuilder&) = delete;

    PipelineAttachmentBuilder& samples(VkSampleCountFlagBits samples);

    PipelineAttachmentBuilder& loadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);

    PipelineAttachmentBuilder& loadStoreStencilOp(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);

    PipelineAttachmentBuilder& layout(VkImageLayout initialLayout, VkImageLayout finalLayout);

    // Redefinitions
    PipelineAttachmentBuilder& redefineFormat(VkFormat format);

    PipelineAttachmentBuilder& redefineRef(uint32_t attachmentId, VkImageLayout layout);
};
