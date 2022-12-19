#pragma once

#include "VulkanRHI.h"

// vulkan一些逻辑数据的存放的地方
// 比如device, physic device等
class UVulkanProxy
{
public:
    UVulkanProxy() = default;

    //方便调用, 提供了原始vk的接口

    VkDevice Device;
    VkPhysicalDevice Gpu;
};
