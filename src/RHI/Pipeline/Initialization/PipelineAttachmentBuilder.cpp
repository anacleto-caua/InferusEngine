#include "PipelineAttachmentBuilder.hpp"

PipelineAttachmentBuilder::PipelineAttachmentBuilder(uint32_t attachmentId, VkImageLayout imageRefLayout, VkFormat format) {
    attachment.description.format = format;
    attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.description.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.reference.attachment = attachmentId;
    attachment.reference.layout = imageRefLayout;
}

PipelineAttachmentBuilder& PipelineAttachmentBuilder::samples(VkSampleCountFlagBits samples) {
    attachment.description.samples = samples;
    return *this;
}

PipelineAttachmentBuilder& PipelineAttachmentBuilder::loadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
    attachment.description.loadOp = loadOp;
    attachment.description.storeOp = storeOp;
    return *this;
}

PipelineAttachmentBuilder& PipelineAttachmentBuilder::loadStoreStencilOp(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
    attachment.description.stencilLoadOp = stencilLoadOp;
    attachment.description.stencilStoreOp = stencilStoreOp;
    return *this;
}

PipelineAttachmentBuilder& PipelineAttachmentBuilder::layout(VkImageLayout initialLayout, VkImageLayout finalLayout) {
    attachment.description.initialLayout = initialLayout;
    attachment.description.finalLayout = finalLayout;
    return *this;
}

// Redefinitions
PipelineAttachmentBuilder& PipelineAttachmentBuilder::redefineFormat(VkFormat format) {
    attachment.description.format = format;
    return *this;
}

PipelineAttachmentBuilder& PipelineAttachmentBuilder::redefineRef(uint32_t attachmentId, VkImageLayout layout) {
    attachment.reference.attachment = attachmentId;
    attachment.reference.layout = layout;
    return *this;
}