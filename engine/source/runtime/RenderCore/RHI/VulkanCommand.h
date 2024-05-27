#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>

class FVulkanDevice;
class FVulkanFrameBuffer;
class FVulkanPipeline;
class FVulkanDescriptorSet;
class FVulkanRenderPass;

class FVulkanCommandBuffer;

class FVulkanCommandPool
{
public:
    FVulkanCommandPool() = default;

    //------ vulkan 对象返回 ---------
    VkCommandPool& GetVkPool() { return RawPool; }
    //------ vulkan 对象返回 ---------

    //TODO 兼容引擎, 目前使用m_rhi创建的pool
    void SetPool(VkCommandPool& InPool) { RawPool = InPool; }

    void CreateCommandPool(FVulkanDevice& Device, uint32_t QueueFamilyIndex);

    void Destroy(FVulkanDevice& Device);
private:
    VkCommandPool RawPool = VK_NULL_HANDLE;
};

class FVulkanCommandBuffer
{
friend class FVulkanCommandPool;

public:
    FVulkanCommandBuffer() = default;

    explicit FVulkanCommandBuffer(VkCommandBuffer& Buffer);

    VkCommandBuffer& GetVkCommandBuffer() { return RawCommandBuffer; }
    
    //申请单个命令, 原始api可以申请多个. 这个参考picco代码, 发现都是单个申请的, 所以这里简化了API
    void AllocateCommandBuffer(FVulkanDevice& Device, FVulkanCommandPool& Pool);

    //对于单次的重操作的指令, 用完后就销毁.
    void Destroy(FVulkanDevice& Device, FVulkanCommandPool& CommandPool);

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
    void BindPipeline(FVulkanPipeline& Pipeline);

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
    VkCommandBuffer RawCommandBuffer = VK_NULL_HANDLE;

    //是否用于单次重操作的目的. 比如copybuffer之类的
    bool bOneTimeSubmit = false;

    bool bInDebug = false;
};
