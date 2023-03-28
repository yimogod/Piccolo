#pragma once

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <stdexcept>

struct FVulkanFrameBufferAttachment;

class FVulkanRHIUtility
{
public:
    FVulkanRHIUtility() = default;

    //根据参数, 获取内存类型的索引
    static uint32_t FindMemoryType(VkPhysicalDeviceMemoryProperties& MemProperties,
                                   uint32_t RequiredMemoryTypeBits,
                                   VkMemoryPropertyFlags RequriedProperty);

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

    //创建帧缓冲用的image
    //帧缓冲用到的imageview可以是交换链提供的, 也可以是程序创建的. 这里提供了手动创建的类
    static void CreateFrameAttachment(FVulkanFrameBufferAttachment& OutAttachment,
                                      VkDevice Device,
                                      VkPhysicalDevice Gpu,
                                      //image 的tiling 方式
                                      VkImageTiling Tiling,
                                      //image 的用处. 比如颜色附件, 深度附件, 用于采样, 用于临时传输之类的
                                      VkImageUsageFlags Usage,
                                      //image 申请的内存块是仅gpu访问,还是cpu/gpu都能访问
                                      VkMemoryPropertyFlags MemoryProperty,
                                      //image view的aspect, 常用的有颜色还有深度
                                      VkImageAspectFlags AspectFlags,
                                      // image view的格式. 一般是2d或者2d array
                                      VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D,
                                      // image view的mip level
                                      uint32_t MipLevelCount = 1,
                                      // image view的层数, 还有一个baselayer, 是起始层数. 我没放开参数. 给了0
                                      uint32_t ArrayLayers = 1
                                      );
};

