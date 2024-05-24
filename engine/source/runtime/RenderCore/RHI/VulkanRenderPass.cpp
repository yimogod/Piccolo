#include "VulkanRenderPass.h"
#include <stdexcept>
#include "vulkan/vulkan_core.h"
#include "VulkanDevice.h"

void FVulkanSubPass::SetReferenceNum(uint32_t InputNum, uint32_t ColorNum, uint32_t DepthNum)
{
    InputReferences.resize(InputNum);
    ColorReferences.resize(ColorNum);
    DepthReferences.resize(DepthNum);
}

void FVulkanSubPass::SetInputReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex, VkImageLayout InLayout)
{
    InputReferences[ReferenceIndex].attachment = InAttachmentIndex;
    InputReferences[ReferenceIndex].layout = InLayout;
}

void FVulkanSubPass::SetColorReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex, VkImageLayout InLayout)
{
    ColorReferences[ReferenceIndex].attachment = InAttachmentIndex;
    ColorReferences[ReferenceIndex].layout = InLayout;
}
void FVulkanSubPass::SetColorReference(uint32_t ReferenceIndex, uint32_t InAttachmentIndex)
{
    ColorReferences[ReferenceIndex].attachment = InAttachmentIndex;
    ColorReferences[ReferenceIndex].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void FVulkanSubPass::SetDepthReference(uint32_t InAttachmentIndex)
{
    DepthReferences[0].attachment = InAttachmentIndex;
    DepthReferences[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void FVulkanSubPass::SetPreserveReference(uint32_t InAttachmentCount, uint32_t InAttachmentIndex)
{
    PreserveCount = InAttachmentCount;
    PreserveIndex = InAttachmentIndex;
}

void FVulkanSubPass::SetupSubPass()
{
    //VkSubpassDescription 很重要. 决定subpass输出到哪儿!!!
    //input attachment是输入的imageview
    //color就是输出的image view

    VKSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VKSubpass.inputAttachmentCount = 0;
    VKSubpass.pInputAttachments = nullptr;
    VKSubpass.colorAttachmentCount = 0;
    VKSubpass.pColorAttachments = nullptr;
    VKSubpass.preserveAttachmentCount = 0;
    VKSubpass.pPreserveAttachments    = nullptr;
    VKSubpass.pDepthStencilAttachment = nullptr;

    if(InputReferences.size() > 0)
    {
        VKSubpass.inputAttachmentCount = InputReferences.size();
        VKSubpass.pInputAttachments = InputReferences.data();
    }

    if(ColorReferences.size() > 0)
    {
        VKSubpass.colorAttachmentCount = ColorReferences.size();
        VKSubpass.pColorAttachments = ColorReferences.data();
    }

    if(DepthReferences.size() > 0)
    {
        VKSubpass.pDepthStencilAttachment = DepthReferences.data();
    }

    if(PreserveCount > 0)
    {
        VKSubpass.preserveAttachmentCount = PreserveCount;
        VKSubpass.pPreserveAttachments = &PreserveIndex;
    }


}


void FVulkanSubPassDependency::SetDependencyNum(uint32_t Num)
{
    Dependencies.resize(Num);
    //设置依赖的默认值
    for (VkSubpassDependency& Dep : Dependencies)
    {
        Dep.srcSubpass = 0;
        Dep.dstSubpass = 0;
        Dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        Dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        Dep.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //源是写?
        Dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; //目标是读
        Dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
}
void FVulkanSubPassDependency::SetFlag(uint32_t DependencyIndex, VkDependencyFlags Flag)
{
    Dependencies[DependencyIndex].dependencyFlags = Flag;
}
void FVulkanSubPassDependency::SetSubpass(uint32_t DependencyIndex, uint32_t SrcSubpass, uint32_t DstSubpass)
{
    Dependencies[DependencyIndex].srcSubpass = SrcSubpass;
    Dependencies[DependencyIndex].dstSubpass = DstSubpass;
}
void FVulkanSubPassDependency::SetStageMask(uint32_t             DependencyIndex,
                                            VkPipelineStageFlags SrcStageMask,
                                            VkPipelineStageFlags DstStageMask)
{
    Dependencies[DependencyIndex].srcStageMask = SrcStageMask;
    Dependencies[DependencyIndex].dstStageMask = DstStageMask;
}
void FVulkanSubPassDependency::SetAccessMask(uint32_t      DependencyIndex,
                                             VkAccessFlags SrcAccessMask,
                                             VkAccessFlags DstAccessMask)
{
    Dependencies[DependencyIndex].srcAccessMask = SrcAccessMask;
    Dependencies[DependencyIndex].dstAccessMask = DstAccessMask;
}


void FVulkanRenderPass::CreateRenderPass(FVulkanDevice& Device)
{
    //组装subpass
    std::vector<VkSubpassDescription> VkSubpasses;
    VkSubpasses.resize(SubPasses.size());
    for(uint32_t i = 0; i < SubPasses.size(); i++)
    {
        VkSubpasses[i] = SubPasses[i].VKSubpass;
    }

    VkRenderPassCreateInfo CreateInfo {};
    CreateInfo.sType                       = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    CreateInfo.attachmentCount             = AttachmentDesc.GetAttachmentNum();
    CreateInfo.pAttachments                = AttachmentDesc.Attachments.data();
    CreateInfo.subpassCount                = SubPasses.size();
    CreateInfo.pSubpasses                  = VkSubpasses.data();
    CreateInfo.dependencyCount             = Dependency.GetDependencyNum();
    CreateInfo.pDependencies               = Dependency.Dependencies.data();

    //vulkan创建render pass
    if (vkCreateRenderPass(Device.GetDevice(), &CreateInfo, nullptr, &VKRenderPass) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}


