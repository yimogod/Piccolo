#pragma once
#include "VulkanDevice.h"
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
    friend class FVulkanImageView;

public:
    FVulkanImage() = default;

    //用于兼容现有引擎使用方式, 在外部已经创建了Buffer
    explicit FVulkanImage(VkImage& InBuffer) { RawImage = InBuffer; }

    VkImage& GetImage() { return RawImage; }

    //创建实际的VulkanImage对象
    void CreateImage(FVulkanDevice& Device);

    void Destroy(FVulkanDevice& Device);

    //经过优化的排列方式
    VkImageTiling Tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;

    //图片的用处, VkImageUsageFlagBits 的值
    VkImageUsageFlags Usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //图片的内存性质, 默认是显存
    VkMemoryPropertyFlags MemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
protected:
    //图片数据
    VkImage RawImage = VK_NULL_HANDLE;
    //存放图片的显存
    VkDeviceMemory RawMemory = VK_NULL_HANDLE;

    //开辟的image的尺寸. 比如平行光的shadow map, 就开辟了4096*4096.
    uint32_t Width = 0;
    uint32_t Height = 0;

    //图片格式
    VkFormat Format = VkFormat::VK_FORMAT_B8G8R8A8_UINT;

    //如果是对象数组的话, 有几层
    uint32_t ArrayLayers = 1;
    //默认标准2d图片
    VkImageType ImageType = VkImageType::VK_IMAGE_TYPE_2D;

};

class FVulkanImage2D : public FVulkanImage
{
public:
    FVulkanImage2D() = default;

    FVulkanImage2D(uint32_t W, uint32_t H, VkFormat InFormat, VkImageUsageFlags InUsage);
};


class FVulkanImageView
{
public:
    FVulkanImageView() = default;

    FVulkanImageView(FVulkanImage& InImage, uint32_t InMipNum = 1);

    VkImageView& GetVkImageView() { return RawView; }

    void CreateImageView_Color(FVulkanDevice& Device);

    void CreateImageView_Cube(FVulkanDevice& Device);

    void CreateImageView_Depth(FVulkanDevice& Device);

    void Destroy(FVulkanDevice& Device);
private:
    //创建实际的VulkanImageView对象
    void InternalCreateImageView(FVulkanDevice& Device, VkImageViewType ViewType, VkImageAspectFlags AspectMask);

    //mip levels
    uint32_t MipNum = 1;

    FVulkanImage Image;
    VkImageView RawView = VK_NULL_HANDLE;
};
