#pragma once
#include <vector>
#include "vulkan/vulkan_core.h"

//这里直接把附件描述作为一个数组维护了. 提供给RenderPass使用
//先将附件粗暴的理解为RT
//附件是对ImageView的描述. 定义RenderPass如何使用这些Image. 也即RenderPass如何绘制到RT
//而附件对应的ImageView在哪里呢? 在FrameBuffer里. 即这里用到了几个附件. 在Frame也得clear多少个
//
//AttachmentDescription提供了本pass所有用到的附件, 然后经过组合使用形成不同的Reference,
//然后一组Reference对应一个subpass, subpass之间有依赖关系
//最终这些附件平铺, subpass及其依赖, 组成了render pass
//
//这些附件也需要和FrameBuffer中的帧缓冲附件对应起来, 其实根儿里面指向的都是同样的ImageView
// ColorAttachmentOutput是单独的shader阶段. 在PS阶段之后. 可以理解为PS输出到了RT
class FVulkanAttachmentDescription
{
    friend class FVulkanRenderPass;

public:
    FVulkanAttachmentDescription() = default;

    //设置附件的格式
    void SetFormat(uint32_t AttachmentIndex, VkFormat Format);

    //设置附件加载和读取的操作标记
    void SetLoadAndStore(uint32_t AttachmentIndex, VkAttachmentLoadOp LoadOp, VkAttachmentStoreOp StoreOp);

    //设置模板附件加载和读取的操作标记
    void SetStencilLoadAndStore(uint32_t AttachmentIndex, VkAttachmentLoadOp LoadOp, VkAttachmentStoreOp StoreOp);

    //设置附件的初始化布局和最终的布局(可能是输出的时候的布局)
    void SetLayout(uint32_t AttachmentIndex, VkImageLayout InitialLayout, VkImageLayout FinalLayout);

private:
    //有几个附件
    uint32_t GetAttachmentNum() {return Attachments.size();}

    //设置RenderPass一共需要用到多少个附件
    void SetAttachmentNum(uint32_t Num);

    //原生vk对象
    std::vector <VkAttachmentDescription> Attachments;
};
