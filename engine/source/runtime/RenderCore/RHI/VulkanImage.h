#pragma once
#include "utility/MathUtility.h"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <vector>

/**
 * 缓冲区对象. 顶点索引unifor缓冲区继承此类
 * 1. 创建缓冲区需要用到的Vulkan对象有Device, PhysicalDevice
 * 2. PhysicalDevice用于查找对应的内存类型
 */
class FVulkanImage
{
public:
    FVulkanImage() = default;
    //用于兼容现有引擎使用方式, 在外部已经创建了Buffer
    explicit FVulkanImage(VkImage& InBuffer) { RawBuffer = InBuffer; }

    VkImage& GetVkBuffer() { return RawBuffer; }
    uint64_t GetBufferSize() { return BufferSize; }

    void Destroy(VkDevice& Device);
protected:
    void InnerCreateBuffer(VkDevice& Device, VkPhysicalDevice& Gpu, VkBufferUsageFlags Usage,
                           VkMemoryPropertyFlags MemoryProperty);

    VkImage       RawBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory RawBufferMemory = VK_NULL_HANDLE;
    //缓存当前buff的大小
    uint64_t BufferSize = 0;
};
