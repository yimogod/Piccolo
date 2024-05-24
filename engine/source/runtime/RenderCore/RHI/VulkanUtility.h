#pragma once

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <stdexcept>

struct FVulkanFrameBufferAttachment;

class FVulkanUtility
{
public:
    FVulkanUtility() = default;

    //根据参数, 获取内存类型的索引
    static uint32_t FindMemoryType(VkPhysicalDeviceMemoryProperties& MemProperties,
                                   uint32_t RequiredMemoryTypeBits,
                                   VkMemoryPropertyFlags MemFlag);

    //分配内存的封装方法
    static VkResult AllocateMemory(VkDevice& Device, VkPhysicalDevice GPU, VkMemoryPropertyFlags MemFlag, VkMemoryRequirements& MemRequire, VkDeviceMemory* pOutMem);



    // 内部创建图片的函数
    // TODO 创建一个Image类来封装这些状态和接口
    static void CreateImage(VkDevice Device,
                            VkPhysicalDevice Gpu,
                            uint32_t Width,
                            uint32_t Height,
                            VkFormat Format,
                            uint32_t ArrayLayers,
                            VkImageTiling Tiling,
                            VkImageUsageFlags Usage,
                            VkMemoryPropertyFlags MemoryProperty,
                            VkImage& OutImage,
                            VkDeviceMemory& OutImageMemory);

    //创建图片视图赋值函数
    static VkImageView CreateImageView(VkDevice Device,
                                       VkImage Image,
                                       VkFormat Format,
                                       VkImageAspectFlags AspectFlags,
                                       VkImageViewType ViewType,
                                       uint32_t MipLevelCount,
                                       uint32_t LayerCount);

    //查找默认支持的深度贴图的格式
    static VkFormat FindDepthFormat(VkPhysicalDevice GPU);
};

