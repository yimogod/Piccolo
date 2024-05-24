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

void FVulkanFrameBuffer::AddAttachment(FVulkanImageView& Attachment)
{
    CachedAttachments.push_back(Attachment);
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

void FVulkanFrameBuffer::CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass, std::vector<VkImageView>& SwapChainView)
{
    CachedFramebuffers.resize(SwapChainView.size());

    // 三缓存, 所以3个frame buffer
    for (size_t i = 0; i < SwapChainView.size(); i++)
    {
        //这个view除了添加之前的普通imageview, 还需要加上交换链的View

        //先设置 非 swapchain的view
        std::vector<VkImageView> Views;
        Views.resize(CachedAttachments.size() + 1);
        for (int j = 0; j < Views.size() - 1; ++j)
        {
            Views[j] = CachedAttachments[j].GetImageView();
        }

        //在设置swapchain的view
        Views[CachedAttachments.size()] = SwapChainView[i];

        //最后创建frame buffer
        CreateFrameBufferInner(Device.GetVkDevice(), RenderPass.GetVkRenderPass(), Views, 1, i);
    }
}

void FVulkanFrameBuffer::CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass)
{
    CreateFrameBuffer(Device, RenderPass, 1);
}

void FVulkanFrameBuffer::CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass, uint32_t FrameLayers)
{
    //普通的pass, 就一个frame buffer
    CachedFramebuffers.resize(1);

    //这个view除了添加之前的普通imageview, 还需要加上交换链的View
    std::vector<VkImageView> View;
    View.resize(CachedAttachments.size());
    for (int i = 0; i < View.size(); ++i)
    {
        View[i] = CachedAttachments[i].GetImageView();
    }

    CreateFrameBufferInner(Device.GetVkDevice(), RenderPass.GetVkRenderPass(), View, FrameLayers, 0);
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
    if(ClearValues.empty())
    {
        ClearValues.resize(ImageViews.size());
    }
}
void FVulkanFrameBuffer::DestroyFrameBuffer(VkDevice& Device)
{
    for (auto* FrameBuffer : CachedFramebuffers)
    {
        vkDestroyFramebuffer(Device, FrameBuffer, nullptr);
    }
}

