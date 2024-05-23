#pragma once

#include "VulkanRHI.h"

// vulkan一些逻辑数据的存放的地方
// 比如device, physic device等
class UVulkanProxy
{
public:
    UVulkanProxy() = default;

    //方便调用, 提供了原始vk的接口
    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice Gpu = VK_NULL_HANDLE;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    //初始化vulkan系统. 比如
    // 1. 描述符池子,
    void Initialized();
};
