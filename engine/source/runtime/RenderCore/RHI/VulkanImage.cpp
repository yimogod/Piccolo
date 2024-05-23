//
// Created by zhibin.liu on 2024/2/1.
//

#include "VulkanImage.h"
#include "VulkanUtility.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <stdexcept>

void FVulkanImage::Destroy(FVulkanDevice& Device)
{
    vkDestroyImage(Device.GetDevice(), RawImage, nullptr);
    vkFreeMemory(Device.GetDevice(), RawMemory, nullptr);
}

void FVulkanImage::CreateImage(FVulkanDevice& Device)
{
    //1. 创建图片对象. 只是个对象的代表, 并没有实际的内存分配
    VkImageCreateInfo ImageInfo{};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = ImageType;
    ImageInfo.extent.width = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth = 1;
    ImageInfo.format = Format;
    ImageInfo.mipLevels = 1;
    ImageInfo.arrayLayers = 1;
    ImageInfo.tiling = Tiling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage = Usage;
    ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(Device.GetDevice(), &ImageInfo, nullptr, &RawImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    //图片需要的显存
    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(Device.GetDevice(), RawImage, &MemRequirements);
    VkResult Result = FVulkanUtility::AllocateMemory(Device.GetDevice(), Device.GetGPU(), MemoryProperty, MemRequirements, &RawMemory);
    if (Result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    //绑定图片对象和对应的显存区域
    vkBindImageMemory(Device.GetDevice(), RawImage, RawMemory, 0);
}

FVulkanImage2D::FVulkanImage2D(uint32_t W, uint32_t H, VkFormat InFormat, VkImageUsageFlags InUsage)
{
    Width = W;
    Height = H;
    Format = InFormat;
    Usage = InUsage;
}

// ----------------------- Image View -----------------------
FVulkanImageView::FVulkanImageView(FVulkanImage& InImage, uint32_t InMipNum)
{
    Image = InImage;
    MipNum = InMipNum;
}

void FVulkanImageView::CreateImageView_Color(FVulkanDevice& Device)
{
    InternalCreateImageView(Device,
                            VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
                            VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
}

void FVulkanImageView::CreateImageView_Cube(FVulkanDevice& Device)
{
    InternalCreateImageView(Device,
                            VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE,
                            VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
}

void FVulkanImageView::CreateImageView_Depth(FVulkanDevice& Device)
{
    InternalCreateImageView(Device,
                            VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
                            VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);
}

void FVulkanImageView::InternalCreateImageView(FVulkanDevice& Device, VkImageViewType ViewType, VkImageAspectFlags AspectMask)
{
    VkImageViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.image = Image.RawImage;
    CreateInfo.format = Image.Format;
    CreateInfo.viewType = ViewType;

    CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.subresourceRange.aspectMask = AspectMask;
    CreateInfo.subresourceRange.baseMipLevel = 0;
    CreateInfo.subresourceRange.levelCount = MipNum;
    CreateInfo.subresourceRange.baseArrayLayer = 0;
    CreateInfo.subresourceRange.layerCount = Image.ArrayLayers;

    if (vkCreateImageView(Device.GetDevice(), &CreateInfo, nullptr, &RawView)!= VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
    }
}

void FVulkanImageView::Destroy(FVulkanDevice& Device)
{
    vkDestroyImageView(Device.GetDevice(), RawView, nullptr);
}

