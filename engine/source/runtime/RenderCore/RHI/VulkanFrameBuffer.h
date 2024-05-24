#pragma once

#include "VulkanImage.h"
#include "VulkanRenderPass.h"
#include "vulkan/vulkan_core.h"
#include <vector>


//framebuffer有两种使用情境
//1是只有一个VkFrameBuffer, 在pioccolo中, 如果不做present, 一个足够
//2是展示到屏幕, 因为用了3缓冲技术, 所以会有三个framebuffer
//除了是VkFrameBuffer的封装外, 还缓存了自已用到的所有的附件
class FVulkanFrameBuffer
{
    friend class FVulkanCommandBuffer;

public:
    FVulkanFrameBuffer() = default;

    //frame buffer尺寸, frame width
    uint32_t Width;
    //frame height
    uint32_t Height;

    //获取对应的FrameBuffer
    VkFramebuffer& GetVkFrameBuffer(uint32_t Index) { return CachedFramebuffers[Index]; }
    //获取当前正在使用的FrameBuffer
    VkFramebuffer& GetVkFrameBuffer() { return CachedFramebuffers[CurrBufferIndex]; }
    //获取附件的格式
    VkFormat GetAttachmentFormat(uint32_t Index){ return CachedAttachments[Index].GetFormat(); }
    //获取附件
    FVulkanImageView& GetAttachment(uint32_t Index){ return CachedAttachments[Index]; }


    //获取frame充满的viewport
    VkViewport GetFullViewport();
    //获取frame充满的裁剪区域
    VkRect2D GetFullScissor();
    //获取整个frame的尺寸
    VkExtent2D GetFullExtent();

    //设置马上要用到的vkframebuffer索引.
    void SetCurrentBufferIndex(uint32_t Index) { CurrBufferIndex = Index; }
    void AddAttachment(FVulkanImageView& Attachment);

    void SetFrameAttachmentNum(uint32_t Num) { CachedAttachments.resize(Num); }

    //设置ClearValue的值
    void SetClearColorValue(uint32_t Index, VkClearColorValue Value);
    void SetClearDepthValue(uint32_t Index, VkClearDepthStencilValue Value);
    void SetClearColors_Black();


    //合并交换链的view, 创建frame buffer, 一般用于最后一个pass, 展示渲染结果
    void CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass, std::vector<VkImageView>& SwapChainView);

    //创建frame buffer. 不 包含交换链的imageview, FrameLayers是VkFramebufferCreateInfo的layers的成员变量
    void CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass);
    void CreateFrameBuffer(FVulkanDevice& Device, FVulkanRenderPass& RenderPass, uint32_t FrameLayers);

    void DestroyFrameBuffer(VkDevice& Device);
private:
    void CreateFrameBufferInner(VkDevice& Device, VkRenderPass& RenderPass, std::vector<VkImageView>& ImageViews, uint32_t FrameLayers, uint32_t BufferIndex);

    uint32_t CurrBufferIndex = 0;

    //本Pass需要的所有的帧缓冲. 一般的中间流程的pass创建一个Framebuffer就好.
    //但最终用于输出到屏幕的需要3个交换链对应的3个Framebuffer. 所以这里
    std::vector<VkFramebuffer> CachedFramebuffers;

    // 自定义的ImageView, maincamerapass用到了7个.
    // 另外加上交换链所需的深度和swapchianimageview, 共9个给renderpas使用

    //用到的所有的帧缓冲附件. 这个附件个数和描述符集数组(即RenderPass用到的所有的附件)所有用到的个数相同
    //附件仅用于PS, 可以粗暴的将附件理解为RenderTarget
    std::vector<FVulkanImageView> CachedAttachments;

    //frame buffer对应的clear value
    std::vector<VkClearValue> ClearValues;
};
