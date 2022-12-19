#include "VulkanCommand.h"
#include "vulkan/vulkan_core.h"
#include "VulkanFrameBuffer.h"

void FVulkanCommandPool::CreateCommandPool(VkDevice& Device)
{

}
void FVulkanCommandPool::Destroy(VkDevice& Device)
{

}

FVulkanCommandBuffer::FVulkanCommandBuffer(VkCommandBuffer& Buffer)
{
    RawCommandBuffer = Buffer;
}
void FVulkanCommandBuffer::AllocateCommandBuffer(VkDevice& Device, VkCommandPool& Pool)
{

}
void FVulkanCommandBuffer::Destroy(VkDevice& Device, VkCommandPool& CommandPool)
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
    BeginInfo.framebuffer       = FrameBuffer.GetFrameBuffer();
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
void FVulkanCommandBuffer::BindPipeline(VkPipeline& Pipeline)
{
    vkCmdBindPipeline(RawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
}
void FVulkanCommandBuffer::SetViewport(VkViewport& Viewport)
{
    vkCmdSetViewport(RawCommandBuffer, 0, 1, &Viewport);
}
void FVulkanCommandBuffer::SetScissor(VkRect2D& Scissor)
{
    vkCmdSetScissor(RawCommandBuffer, 0, 1, &Scissor);
}
void FVulkanCommandBuffer::BindVertexBuffer(VkBuffer& Buffer)
{
    VkDeviceSize Offsets[] = { 0 };
    //auto BufferData = VertexBuffer.GetVkData();
    vkCmdBindVertexBuffers(RawCommandBuffer,
                           0, //顶点数据缓冲在列表中的索引
                           1, //顶点缓冲的数量
                           &Buffer, //顶点数据缓冲地址
                           Offsets //各个顶点数据缓冲的内部偏移量
    );
}
void FVulkanCommandBuffer::BindIndexBuffer(VkBuffer& Buffer)
{
    vkCmdBindIndexBuffer(RawCommandBuffer,
                         Buffer,
                         0, //各个索引数据在缓冲区的内部偏移
                         VK_INDEX_TYPE_UINT16 //因为demo我们的定点数肯定小于65536, 所以用了uint16, 可以省一半内存
    );
}
void FVulkanCommandBuffer::BindDescriptorSets(VkPipelineLayout& PipelineLayout, VkDescriptorSet& DescriptorSet, uint32_t FirstSet)
{
    vkCmdBindDescriptorSets(RawCommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            PipelineLayout,
                            FirstSet,
                            1,
                            &DescriptorSet,
                            0,
                            nullptr);
}
void FVulkanCommandBuffer::BindDescriptorSets(VkPipelineLayout& PipelineLayout, VkDescriptorSet& DescriptorSet, std::vector<uint32_t>& DynamicOffsets)
{
    vkCmdBindDescriptorSets(RawCommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            PipelineLayout,
                            0,
                            1,
                            &DescriptorSet,
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
