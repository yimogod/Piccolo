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


    //默认的深度格式
    VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;

    FVulkanDevice Device2;
    FVulkanDescriptorPool DescriptorPool2;

    FVulkanCommandPool CmdPool;
    std::vector<FVulkanCommandBuffer> CmdBuffers;
    FVulkanCommandBuffer CurrCmdBuffer;

    //初始化vulkan系统. 比如
    // 1. 描述符池子,
    void Initialized();
};
