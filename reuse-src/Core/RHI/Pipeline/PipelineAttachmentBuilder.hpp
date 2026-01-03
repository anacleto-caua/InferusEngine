#pragma once

#include <vulkan/vulkan.h>

class PipelineAttachmentBuilder {
private:
    struct AttachmentBuild {
        VkAttachmentDescription description{};
        VkAttachmentReference reference{};
    };

public:
    AttachmentBuild attachment;

    static PipelineAttachmentBuilder setDefaults(uint32_t attachmentId, VkImageLayout imageRefLayout, VkFormat format) {
        PipelineAttachmentBuilder builder;

        builder.attachment.description.format = format;
        builder.attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
        builder.attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        builder.attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        builder.attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        builder.attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        builder.attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        builder.attachment.description.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        builder.attachment.reference.attachment = attachmentId;
        builder.attachment.reference.layout = imageRefLayout;

        return builder;
    }

    PipelineAttachmentBuilder& samples(VkSampleCountFlagBits samples) {
        attachment.description.samples = samples;
        return *this;
    }

    PipelineAttachmentBuilder& loadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
        attachment.description.loadOp = loadOp;
        attachment.description.storeOp = storeOp;
        return *this;
    }

    PipelineAttachmentBuilder& loadStoreStencilOp(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
        attachment.description.stencilLoadOp = stencilLoadOp;
        attachment.description.stencilStoreOp = stencilStoreOp;
        return *this;
    }

    PipelineAttachmentBuilder& layout(VkImageLayout initialLayout, VkImageLayout finalLayout) {
        attachment.description.initialLayout = initialLayout;
        attachment.description.finalLayout = finalLayout;
        return *this;
    }

    // Redefinitions
    PipelineAttachmentBuilder& redefineFormat(VkFormat format) {
        attachment.description.format = format;
        return *this;
    }

    PipelineAttachmentBuilder& redefineRef(uint32_t attachmentId, VkImageLayout layout) {
        attachment.reference.attachment = attachmentId;
        attachment.reference.layout = layout;
        return *this;
    }
};