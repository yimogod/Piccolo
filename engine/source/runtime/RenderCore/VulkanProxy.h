#pragma once

#include "VulkanRHI.h"

// vulkan一些逻辑数据的存放的地方
// 比如device, physic device等
// 目前和原生的vulkan_rhi基本对应
class UVulkanProxy
{
public:
    //三缓冲
    static uint8_t const SMaxFramesInFlight {3};


    UVulkanProxy() = default;

    //TODO 用就rhi的数据. 日后会删除
    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice Gpu = VK_NULL_HANDLE;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    VkCommandPool CmdPool2 = VK_NULL_HANDLE;
    //TODO -----------------------------------


    //三缓冲的索引
    uint8_t CurrFrameIndex = 0;

    //默认的深度格式
    VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;

    FVulkanDevice Device2;
    FVulkanDescriptorPool DescriptorPool2;

    FVulkanCommandPool CmdPool;

    //
    bool m_enable_debug_utils_label = true;

    //获取当前的command buffer
    FVulkanCommandBuffer& GetCmdBuffer(){ return CurrCmdBuffer; }

    FVulkanCommandBuffer& GetCmdBuffer(uint32_t Index){ return CmdBuffers[Index]; }

    //初始化vulkan系统. 比如
    // 1. 描述符池子,
    void Initialized();

    //帧开始, 需要重新准备一些换吧. 比如这里是切换CommandBuffer
    void PrepareContext();


    // 一些调试函数
    //vulkan 打印命令标签
    void PushEvent(std::string name, const float* color);
private:
    std::vector<FVulkanCommandBuffer> CmdBuffers;
    FVulkanCommandBuffer CurrCmdBuffer;
};
