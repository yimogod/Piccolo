#include "VulkanFrameBuffer.h"
#include <stdexcept>

VkViewport FVulkanFrameBuffer::GetFullViewport()
{
    VkViewport Viewport = {0.0,
                           0.0,
                           static_cast<float>(Width),
                           static_cast<float>(Height),
                           0.0,
                           1.0};
    return Viewport;
}
VkRect2D FVulkanFrameBuffer::GetFullScissor()
{
    VkRect2D Scissor  = {0, 0, Width, Height};
    return Scissor;
}

VkExtent2D FVulkanFrameBuffer::GetFullExtent()
{
    VkExtent2D Rect = {Width, Height};
    return Rect;
}

void FVulkanFrameBuffer::AddFrameAttachment(FVulkanFrameBufferAttachment& Attachment)
{
    CachedAttachments.push_back(Attachment);
}
void FVulkanFrameBuffer::SetFrameAttachmentFormat(uint32_t Index, VkFormat Format)
{
    CachedAttachments[Index].Format = Format;
    CachedAttachments[Index].Width  = Width;
    CachedAttachments[Index].Height = Height;
}
void FVulkanFrameBuffer::SetFrameAttachmentView(uint32_t Index, const VkImageView& AttachmentView)
{
    CachedAttachments[Index].View = AttachmentView;
}
void FVulkanFrameBuffer::SetClearColorValue(uint32_t Index, VkClearColorValue Value)
{
    ClearValues[Index].color = Value;
}
void FVulkanFrameBuffer::SetClearDepthValue(uint32_t Index, VkClearDepthStencilValue Value)
{
    ClearValues[Index].depthStencil = Value;
}
void FVulkanFrameBuffer::SetClearColors_Black()
{
    for (auto& Value : ClearValues)
    {
        Value.color = {{0.0f, 0.0f, 0.0f,1.0f}};
    }
}
void FVulkanFrameBuffer::CreateFrameBuffer(VkDevice& Device, VkRenderPass& RenderPass, std::vector<VkImageView>& SwapChainView)
{
    CachedFramebuffers.resize(SwapChainView.size());

    // 三缓存, 所以3个frame buffer
    for (size_t i = 0; i < SwapChainView.size(); i++)
    {
        //这个view除了添加之前的普通imageview, 还需要加上交换链的View
        std::vector<VkImageView> View;
        View.resize(CachedAttachments.size() + 1);
        for (int j = 0; j < View.size() - 1; ++j)
        {
            View[j] = CachedAttachments[j].View;
        }
        View[CachedAttachments.size()] = SwapChainView[i];

        CreateFrameBufferInner(Device, RenderPass, View, 1, i);
    }
}
void FVulkanFrameBuffer::CreateFrameBuffer(VkDevice& Device, VkRenderPass& RenderPass)
{
    CreateFrameBuffer(Device, RenderPass, 1);
}
void FVulkanFrameBuffer::CreateFrameBuffer(VkDevice& Device, VkRenderPass& RenderPass, uint32_t FrameLayers)
{
    //普通的pass, 就一个frame buffer
    CachedFramebuffers.resize(1);

    //这个view除了添加之前的普通imageview, 还需要加上交换链的View
    std::vector<VkImageView> View;
    View.resize(CachedAttachments.size());
    for (int i = 0; i < View.size(); ++i)
    {
        View[i] = CachedAttachments[i].View;
    }

    CreateFrameBufferInner(Device, RenderPass, View, FrameLayers, 0);
}
void FVulkanFrameBuffer::CreateFrameBufferInner(VkDevice&                 Device,
                                                VkRenderPass&             RenderPass,
                                                std::vector<VkImageView>& ImageViews,
                                                uint32_t FrameLayers,
                                                uint32_t BufferIndex)
{
    VkFramebufferCreateInfo CreateInfo {};
    CreateInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    CreateInfo.flags      = 0U;
    CreateInfo.renderPass = RenderPass;
    CreateInfo.attachmentCount = ImageViews.size();
    CreateInfo.pAttachments = ImageViews.data();
    CreateInfo.width        = Width;
    CreateInfo.height       = Height;
    CreateInfo.layers       = FrameLayers;

    //vulkan 创建帧缓冲,
    if (vkCreateFramebuffer(Device, &CreateInfo, nullptr, &CachedFramebuffers[BufferIndex]) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("create framebuffer error");
    }

    //创建Framebuffer对应的clear value
    if(ClearValues.size() == 0)
    {
        ClearValues.resize(ImageViews.size());
    }
}
void FVulkanFrameBuffer::DestroyFrameBuffer(VkDevice& Device)
{
    for (auto FrameBuffer : CachedFramebuffers)
    {
        vkDestroyFramebuffer(Device, FrameBuffer, nullptr);
    }
}
