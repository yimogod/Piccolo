#include "VulkanRenderPass.h"
#include <stdexcept>
#include "vulkan/vulkan_core.h"


void FVulkanAttachmentDescription::SetAttachmentNum(uint32_t Num)
{
    Attachments.resize(Num);

    //设置附件的默认值
    for (VkAttachmentDescription& Attachment : Attachments)
    {
        Attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        //默认值全是do not care 或者 undefined
        Attachment.loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        Attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        Attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        Attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        Attachment.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

void FVulkanAttachmentDescription::SetFormat(uint32_t AttachmentIndex, VkFormat Format)
{
    VkAttachmentDescription& Attachment = Attachments[AttachmentIndex];
    Attachment.format = Format;
}

void FVulkanAttachmentDescription::SetLoadAndStore(uint32_t AttachmentIndex, VkAttachmentLoadOp LoadOp, VkAttachmentStoreOp StoreOp)
{
    //LOAD_OP_LOAD：从system加载Attachment到Tile(即chip memory)
    //LOAD_OP_CLEAR：清理Tile缓冲区(chip memory)的数据
    //LOAD_OP_DONT_CARE：不对Tile缓冲区的数据做任何操作，通常用于Tile内的数据会被全部重新，效率高于LOAD_OP_CLEAR。
    //以上3个标记执行的效率：LOAD_OP_DONT_CARE > LOAD_OP_CLEAR > LOAD_OP_LOAD

    //load op代表从system memory到chip memory
    //store op代表从chip memory到system memory

    //do not care 说明chip和system没有交互, 会提高性能
    VkAttachmentDescription& Attachment = Attachments[AttachmentIndex];
    Attachment.loadOp = LoadOp;
    Attachment.storeOp = StoreOp;
}

void FVulkanAttachmentDescription::SetStencilLoadAndStore(uint32_t             AttachmentIndex,
                                                          VkAttachmentLoadOp  LoadOp,
                                                          VkAttachmentStoreOp StoreOp)
{
    VkAttachmentDescription& Attachment = Attachments[AttachmentIndex];
    Attachment.stencilLoadOp = LoadOp;
    Attachment.stencilStoreOp = StoreOp;
}

void FVulkanAttachmentDescription::SetLayout(uint32_t       AttachmentIndex,
                                             VkImageLayout InitialLayout,
                                             VkImageLayout FinalLayout)
{
    VkAttachmentDescription& Attachment = Attachments[AttachmentIndex];
    Attachment.initialLayout = InitialLayout;
    Attachment.finalLayout = FinalLayout;
}

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

void FVulkanSubPass::CreateSubPass()
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


void FVulkanRenderPass::SetupRenderPass(VkDevice Device)
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
    if (vkCreateRenderPass(Device, &CreateInfo, nullptr, &VKRenderPass) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass");
    }
}


