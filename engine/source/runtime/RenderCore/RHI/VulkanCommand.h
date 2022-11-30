#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>

class FVulkanFrameBuffer;
class FVulkanPipeline;
class FVulkanDescriptorSet;
class FVulkanRenderPass;

class FVulkanCommandBuffer;

class FVulkanCommandPool
{
public:
    FVulkanCommandPool() = default;

    void CreateCommandPool(VkDevice& Device);

    void Destroy(VkDevice& Device);

    //------ vulkan 对象返回 ---------
    VkCommandPool& GetPool() { return RawCommandPool; }
    //------ vulkan 对象返回 ---------
private:
    VkCommandPool RawCommandPool = VK_NULL_HANDLE;
};

class FVulkanCommandBuffer
{
friend class FVulkanCommandPool;

public:
    FVulkanCommandBuffer() = default;

    explicit FVulkanCommandBuffer(VkCommandBuffer& Buffer);

    VkCommandBuffer& GetCommandBuffer() { return RawCommandBuffer; }
    
    //申请单个命令, 原始api可以申请多个. 这个参考picco代码, 发现都是单个申请的, 所以这里简化了API
    void AllocateCommandBuffer(VkDevice& Device, VkCommandPool& Pool);

    //对于单次的重操作的指令, 用完后就销毁.
    void Destroy(VkDevice& Device, VkCommandPool& CommandPool);

    //开始指令操作
    void Begin();

    //结束指令
    void End();

    void SubmitSync(VkQueue GraphicsQueue);

    void SubmitAsync(VkDevice& Device, VkSemaphore& WaitSemaphore, VkSemaphore& SignalSemaphore, VkFence& Fence);

    //开始debug, 这两个方法回报链接错误
    void BeginDebugUtils(VkDebugUtilsLabelEXT& LabelTxt);
    void EndDebugUtils();

    // ---------- 开始具体的指令 -------------
    //开始渲染通道
    void BeginRenderPass(FVulkanFrameBuffer& FrameBuffer, FVulkanRenderPass& RenderPass, VkExtent2D VkExtent);
    //结束渲染通道
    void EndRenderPass();

    //绑定管线
    void BindPipeline(VkPipeline& Pipeline);

    //设置视口
    void SetViewport(VkViewport& Viewport);

    //设置裁剪区
    void SetScissor(VkRect2D& Scissor);

    //绑定顶点缓冲区
    void BindVertexBuffer(VkBuffer& Buffer);
    
	//绑定索引缓冲区
    void BindIndexBuffer(VkBuffer& Buffer);

    //绑定描述符集 
    void BindDescriptorSets(VkPipelineLayout& PipelineLayout, VkDescriptorSet& DescriptorSet, uint32_t FirstSet = 0);
    //绑定描述符集
    void BindDescriptorSets(VkPipelineLayout& PipelineLayout, VkDescriptorSet& DescriptorSet, std::vector<uint32_t>& DynamicOffsets);

    //绘制指令
    void DrawIndexed(uint32_t IndexNum, uint32_t InstanceNum = 1);

    //绘制指令
    void Draw(uint32_t VertextNum);

    // ------------ 单次操作 -----------
    //单次操作, 拷贝buffer
    //void CopyBuffer(FVulkanBuffer& SrcBuffer, FVulkanBuffer& DestBuffer);
private:
    //原生命令缓冲区
    VkCommandBuffer RawCommandBuffer;

    //是否用于单次重操作的目的. 比如copybuffer之类的
    bool bOneTimeSubmit = false;

    bool bInDebug = false;
};
