#include "VulkanDevice.h"

VkResult FVulkanDevice::CreateImage(VkImageCreateInfo* pCreateInfo, VkImage* pImage)
{
    return vkCreateImage(Device, pCreateInfo, nullptr, pImage);
}

void FVulkanDevice::DestroyImage(VkImage& Image)
{
    vkDestroyImage(Device, Image, nullptr);
}

VkResult FVulkanDevice::CreateImageView(VkImageViewCreateInfo* pCreateInfo, VkImageView* pView)
{
    return vkCreateImageView(Device, pCreateInfo, nullptr, pView);
}

void FVulkanDevice::DestroyImageView(VkImageView& View)
{
    vkDestroyImageView(Device, View, nullptr);
}

void FVulkanDevice::FreeMemory(VkDeviceMemory& Memory)
{
    vkFreeMemory(Device, Memory, nullptr);
}

