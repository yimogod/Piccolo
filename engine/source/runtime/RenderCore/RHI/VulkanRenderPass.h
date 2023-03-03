#pragma once

#include "VulkanDescriptor.h"
#include "vulkan/vulkan_core.h"
#include <vector>

class FVulkanCommandBuffer;
class FVulkanRenderPass;

//这里直接把附件描述作为一个数组维护了
//附件就认为是对ImageView的描述. 定义RenderPass如何使用这些Image.
//而附件对应的ImageView在哪里呢? 在FrameBuffer里. 即这里用到了几个附件. 在Frame也得clear多少个

//AttachmentDescription提供了本pass所有用到的附件, 然后经过组合使用形成不同的Reference,
//然后一组Reference对应一个subpass, subpass之间有依赖关系
//最终这些附件平铺, subpass及其依赖, 组成了render pass

//这些附件也需要和FrameBuffer中的帧缓冲附件对应起来, 其实根儿里面指向的都是同样的ImageView

//ColorAttachmentOutput是单独的shader阶段. 在PS阶段之后. 可以理解为PS输出到了RT
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

//将 VkSubpassDescription 以及相关数据抽象成为了子通道
class FVulkanSubPass
{
    friend class FVulkanRenderPass;
    friend class FVulkanCommandBuffer;

public:
    FVulkanSubPass() = default;

    //对三种reference设置数量, 看本subpass需要几个输入附件, 几个输出附件. 以及是否有深度附件
    void SetReferenceNum(uint32_t InputNum, uint32_t ColorNum, uint32_t DepthNum);

    //给input附件设置Layout
    void SetInputReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex, VkImageLayout InLayout);

    //给输出附件设置Layout
    void SetColorReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex, VkImageLayout InLayout);
    //给输出附件设置Layout, 默认给值 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    void SetColorReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex);

    //给深度附件设置Layout, 深度的布局应该全部都是 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    void SetDepthReference(uint32_t InAttachmentIndex);

    //给Preserve附件设置Layout, 从引擎源代码, 只有imgui的subpass用到了Preserve的数据, 且Count是1
    void SetPreserveReference(uint32_t InAttachmentCount, uint32_t InAttachmentIndex);

    //填充完Reference, 创建SubPass对象
    void SetupSubPass();
private:
    //subpass的ColorAttachment
    std::vector <VkAttachmentReference> ColorReferences;

    //subpass的InputAttachment
    std::vector <VkAttachmentReference> InputReferences;

    //subpass的DepthAttachment
    std::vector <VkAttachmentReference> DepthReferences;

    //Preserve附件, 参数设置和输入和颜色附件不一样
    int32_t PreserveCount = 0;
    uint32_t PreserveIndex = 0;

    VkSubpassDescription VKSubpass;
};

//将子通道的依赖数组抽象为一个对象
class FVulkanSubPassDependency
{
    friend class FVulkanRenderPass;
public:
    FVulkanSubPassDependency() = default;

    //设置附件的格式
    void SetFlag(uint32_t DependencyIndex, VkDependencyFlags Flag);

    //依赖双方
    void SetSubpass(uint32_t DependencyIndex, uint32_t SrcSubpass, uint32_t DstSubpass);

    //依赖双方分别用于那个阶段 比如ps阶段, 附件输出阶段等
    void SetStageMask(uint32_t DependencyIndex, VkPipelineStageFlags SrcStageMask, VkPipelineStageFlags DstStageMask);

    //设置依赖的双方如何别访问
    void SetAccessMask(uint32_t DependencyIndex, VkAccessFlags SrcAccessMask, VkAccessFlags DstAccessMask);
private:
    //有几个依赖
    uint32_t GetDependencyNum() { return Dependencies.size(); }

    //设置RenderPass一共需要用到多少个子通道
    void SetDependencyNum(uint32_t Num);

    //原生vk对象
    std::vector <VkSubpassDependency> Dependencies;
};

// RenderPass表示一次渲染目标. 可以Render到屏幕, 也可以渲染到缓冲区
//一个Pass可以包含多个SubPass
class FVulkanRenderPass
{
    friend class FVulkanFrameBuffer;
    friend class FVulkanCommandBuffer;

public:
    FVulkanRenderPass() = default;

    FVulkanAttachmentDescription& GetAttachmentDesc() { return AttachmentDesc; }

    FVulkanSubPassDependency& GetDependency() { return Dependency; }

    VkRenderPass& GetVKRenderPass() { return VKRenderPass; }

    //根据已经配置好的数据,创建vkRenderPass
    void CreateRenderPass(VkDevice Device);

    //设置附件个数
    void SetAttachmentNum(uint32_t Num) { AttachmentDesc.SetAttachmentNum(Num); }

    //设置依赖个数
    void SetDependencyNum(uint32_t Num) { Dependency.SetDependencyNum(Num); }

    //设置子通道个数
    void SetSubPassNum(uint32_t Num) { SubPasses.resize(Num); }

    FVulkanSubPass& GetSubPass(uint32_t Index) { return SubPasses[Index]; }

protected:
    //本pass用到的所有的平铺的附件
    FVulkanAttachmentDescription AttachmentDesc;

    //用到的所有子通道
    std::vector<FVulkanSubPass> SubPasses;

    //数组
    FVulkanSubPassDependency Dependency;

    //原生vk对象
    VkRenderPass VKRenderPass {VK_NULL_HANDLE};
};
