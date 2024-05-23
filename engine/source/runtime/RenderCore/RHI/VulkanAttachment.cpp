#include "VulkanAttachment.h"
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