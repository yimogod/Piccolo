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
    VkFormat GetFormat() { return RawFormat; }
    uint32_t GetWidth() { return Width; }
    uint32_t GetHeight() { return Height; }

    //设置图片尺寸
    void SetSize(uint32_t InWidth, uint32_t InHeight);

    //创建实际的VulkanImage对象
    void CreateImage(FVulkanDevice& Device);

    void Destroy(FVulkanDevice& Device);

    //经过优化的排列方式
    VkImageTiling Tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;

    //图片的用处, VkImageUsageFlagBits 的值
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT 该图像可以被用作一个传输命令的源。
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT 该图像可以被用作一个传输命令的目标。
    // VK_IMAGE_USAGE_SAMPLED_BIT 该图像可以被用于创建一个VkImageView，
    //   用于占用一个类型为 VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE 或 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    //   的VkDescriptorSet槽，并被一个shader采样。
    // VK_IMAGE_USAGE_STORAGE_BIT该图像可以用于创建一个VkImageView，
    //   用于占用VkDescriptorSet的类型为 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE 的一个槽。
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT该图像可以被用于创建一个VkImageView
    //   作为一个VkFramebuffer中的一个color或resolve附件。
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT指出该图像可以用于创建一个VkImageView，
    //   用作一个VkFramebuffer中的 depth/stencil 或 depth/stencil resolve 附件。
    // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT具体实现可能会支持使用
    //   VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT 的内存分配，以使用此usage后备一副图像。
    //   表示图形数据的生存周期很短. 因此没必要写入设备内存
    //   对于任一可以用于创建一个用作color，resolve，depth/stencil，或input附件的VkImageView的图像，
    //   可以设置此bit.
    // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT指出该图像可以被用于创建一个VkImageView，
    //   用于占用VkDescriptorSet的一个类型为VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT的槽；
    //   可以从一个shader中作为一个input附件被读取，并且在一个framebuffer中被用作一个input附件。
    VkImageUsageFlags Usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //图片的内存性质, 默认是显存
    VkMemoryPropertyFlags MemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
protected:
    //图片数据
    VkImage RawImage = VK_NULL_HANDLE;
    //存放图片的显存
    VkDeviceMemory RawMemory = VK_NULL_HANDLE;
    //图片格式
    VkFormat RawFormat = VkFormat::VK_FORMAT_B8G8R8A8_UINT;

    //开辟的image的尺寸. 比如平行光的shadow map, 就开辟了4096*4096.
    uint32_t Width = 0;
    uint32_t Height = 0;

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

/**
 * vkImageView的对应对象.
 * */
class FVulkanImageView
{
public:
    FVulkanImageView() = default;

    FVulkanImageView(FVulkanImage& InImage, uint32_t InMipNum = 1);

    VkImageView& GetVkView() { return RawView; }
    VkImage& GetVkImage() { return Image.RawImage; }
    VkDeviceMemory& GetVkMem() { return Image.RawMemory; }
    VkFormat GetFormat() { return Image.RawFormat; }
    uint32_t GetWidth() { return Image.Width; }
    uint32_t GetHeight() { return Image.Height; }

    //设置图片尺寸
    void SetSize(uint32_t InWidth, uint32_t InHeight);

    //设置图片的format
    void SetFormat(VkFormat InFormat);

    //设置图片的usage
    void SetUsage(VkImageUsageFlags Usage);

    void CreateImageView_Color(FVulkanDevice& Device);

    void CreateImageView_Cube(FVulkanDevice& Device);

    void CreateImageView_Depth(FVulkanDevice& Device);

    void Destroy(FVulkanDevice& Device);
private:
    //创建实际的VulkanImageView对象
    void InternalCreateImageView(FVulkanDevice& Device, VkImageViewType ViewType, VkImageAspectFlags AspectMask);

    //如果view还没有绑定的Image, 那么同时创建Image
    void CreateImageIfHasNot(FVulkanDevice& Device);

    //mip levels
    uint32_t MipNum = 1;

    FVulkanImage Image;
    VkImageView RawView = VK_NULL_HANDLE;
};
