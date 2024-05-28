#include "VulkanCommand.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

void FVulkanCommandPool::CreateCommandPool(FVulkanDevice& Device, uint32_t QueueFamilyIndex)
{
    VkCommandPoolCreateInfo CreateInfo {};
    CreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CreateInfo.pNext            = nullptr;
    CreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CreateInfo.queueFamilyIndex = QueueFamilyIndex;

    if (vkCreateCommandPool(Device.GetVkDevice(), &CreateInfo, nullptr, &RawPool) != VK_SUCCESS)
    {
        throw std::runtime_error("vk create command pool error");
    }
}

void FVulkanCommandPool::Destroy(FVulkanDevice& Device)
{

}

FVulkanCommandBuffer::FVulkanCommandBuffer(VkCommandBuffer& Buffer)
{
    RawCommandBuffer = Buffer;
}

void FVulkanCommandBuffer::AllocateCommandBuffer(FVulkanDevice& Device, FVulkanCommandPool& CommandPool)
{
    VkCommandBufferAllocateInfo CreateInfo {};
    CreateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CreateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CreateInfo.commandBufferCount = 1U;

    CreateInfo.commandPool = CommandPool.GetVkPool();
    if (vkAllocateCommandBuffers(Device.GetVkDevice(), &CreateInfo, &RawCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("allocate command buffers Error");
    }
}

void FVulkanCommandBuffer::Destroy(FVulkanDevice& Device, FVulkanCommandPool& CommandPool)
{

}

void FVulkanCommandBuffer::Begin()
{

}

void FVulkanCommandBuffer::End()
{

}


void FVulkanCommandBuffer::SubmitSync(VkQueue GraphicsQueue)
{

}
void FVulkanCommandBuffer::SubmitAsync(VkDevice&    Device,
                                       VkSemaphore& WaitSemaphore,
                                       VkSemaphore& SignalSemaphore,
                                       VkFence&     Fence)
{

}
void FVulkanCommandBuffer::BeginDebugUtils(VkDebugUtilsLabelEXT& LabelTxt)
{
    if(bInDebug)return;
    bInDebug = true;
    //会报链接错误
    //vkCmdBeginDebugUtilsLabelEXT(RawCommandBuffer, &LabelTxt);
}
void FVulkanCommandBuffer::EndDebugUtils()
{
    if(!bInDebug)return;
    bInDebug = false;
    //会报链接错误
    //vkCmdEndDebugUtilsLabelEXT(RawCommandBuffer);
}

void FVulkanCommandBuffer::BeginRenderPass(FVulkanFrameBuffer& FrameBuffer, FVulkanRenderPass& RenderPass, VkExtent2D VkExtent)
{
    VkRenderPassBeginInfo BeginInfo {};
    BeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass        = RenderPass.VKRenderPass;
    BeginInfo.framebuffer       = FrameBuffer.GetVkFrameBuffer();
    BeginInfo.renderArea.offset = {0, 0};
    BeginInfo.renderArea.extent = VkExtent;
    BeginInfo.clearValueCount = FrameBuffer.ClearValues.size();
    BeginInfo.pClearValues    = FrameBuffer.ClearValues.data();

    vkCmdBeginRenderPass(RawCommandBuffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void FVulkanCommandBuffer::EndRenderPass()
{
    //if (!bRunning)return;
    vkCmdEndRenderPass(RawCommandBuffer);
}

void FVulkanCommandBuffer::BindPipeline(FVulkanPipeline& Pipeline)
{
    vkCmdBindPipeline(RawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetVkPipeline());
}

void FVulkanCommandBuffer::SetViewport(VkViewport& Viewport)
{
    vkCmdSetViewport(RawCommandBuffer, 0, 1, &Viewport);
}
void FVulkanCommandBuffer::SetScissor(VkRect2D& Scissor)
{
    vkCmdSetScissor(RawCommandBuffer, 0, 1, &Scissor);
}
void FVulkanCommandBuffer::BindVertexBuffer(FVulkanBuffer& Buffer)
{
    VkDeviceSize Offsets[] = { 0 };
    VkBuffer& BufferData = Buffer.GetVkBuffer();
    vkCmdBindVertexBuffers(RawCommandBuffer,
                           0, //顶点数据缓冲在列表中的索引
                           1, //顶点缓冲的数量
                           &BufferData, //顶点数据缓冲地址
                           Offsets //各个顶点数据缓冲的内部偏移量
    );
}

void FVulkanCommandBuffer::BindIndexBuffer(FVulkanBuffer& Buffer)
{
    vkCmdBindIndexBuffer(RawCommandBuffer,
                         Buffer.GetVkBuffer(),
                         0, //各个索引数据在缓冲区的内部偏移
                         VK_INDEX_TYPE_UINT16 //因为demo我们的定点数肯定小于65536, 所以用了uint16, 可以省一半内存
    );
}

void FVulkanCommandBuffer::BindDescriptorSets(FVulkanPipeline& Pipeline, FVulkanDescriptorSet& DescriptorSet, uint32_t FirstSet)
{
    vkCmdBindDescriptorSets(RawCommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            Pipeline.GetVkLayout(),
                            FirstSet,
                            1,
                            &DescriptorSet.GetVkDescriptorSet(),
                            0,
                            nullptr);
}

void FVulkanCommandBuffer::BindDescriptorSets(FVulkanPipeline& Pipeline, FVulkanDescriptorSet& DescriptorSet, std::vector<uint32_t>& DynamicOffsets)
{
    vkCmdBindDescriptorSets(RawCommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            Pipeline.GetVkLayout(),
                            0,
                            1,
                            &DescriptorSet.GetVkDescriptorSet(),
                            DynamicOffsets.size(),
                            DynamicOffsets.data());
}

void FVulkanCommandBuffer::DrawIndexed(uint32_t IndexNum, uint32_t InstanceNum)
{
    vkCmdDrawIndexed(RawCommandBuffer,
                     IndexNum, //索引的数量
                     InstanceNum, //实例个数?
                     0, //第一个值的索引, 用于绘制局部数据
                     0, //顶点偏移
                     0 //第一个实例的索引?
    );

}
void FVulkanCommandBuffer::Draw(uint32_t VertextNum)
{
    vkCmdDraw(RawCommandBuffer, 3, 1, 0, 0);
}
