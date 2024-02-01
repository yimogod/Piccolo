#include "VulkanUtility.h"
#include "VulkanFrameBuffer.h"

uint32_t FVulkanUtility::FindMemoryType(VkPhysicalDeviceMemoryProperties& MemProperties,
                               uint32_t RequiredMemoryTypeBits,
                               VkMemoryPropertyFlags RequiredFlag)
{
    for (uint32_t i = 0; i < MemProperties.memoryTypeCount; i++) {
        if ((RequiredMemoryTypeBits & (1 << i)) &&
            (MemProperties.memoryTypes[i].propertyFlags & RequiredFlag) == RequiredFlag) {
            //std::cout << "found Memory Property " << RequriedProperty << " at index " << i << std::endl;
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
    return -1;
}

void FVulkanUtility::CreateImage(VkDevice Device,
                                    VkPhysicalDevice Gpu,
                                    uint32_t Width,
                                    uint32_t Height,
                                    VkFormat Format,
                                    uint32_t ArrayLayers,
                                    VkImageTiling Tiling,
                                    VkImageUsageFlags Usage,
                                    VkMemoryPropertyFlags MemoryProperty,
                                    VkImage& OutImage,
                                    VkDeviceMemory& OutImageMemory) {

    //image 创建参数
    VkImageCreateInfo ImageInfo{};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth = 1;
    ImageInfo.format = Format;
    ImageInfo.mipLevels = 1;
    ImageInfo.arrayLayers = ArrayLayers;
    ImageInfo.tiling = Tiling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage = Usage;
    ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(Device, &ImageInfo, nullptr, &OutImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }


    //图片需要的内存
    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(Device, OutImage, &MemRequirements);

    //开辟显存空间
    //获取显卡的显存的属性. 包含显存类型(包含内存类型和对应的堆索引)和
    //显存堆数据(包含堆大小和堆类型--一般都是device_local)
    VkPhysicalDeviceMemoryProperties MemProperties;
    vkGetPhysicalDeviceMemoryProperties(Gpu, &MemProperties);

    uint32_t MemoryTypeIndex = FVulkanUtility::FindMemoryType(MemProperties,
                                                                 MemRequirements.memoryTypeBits,
                                                                 MemoryProperty);

    //显存开辟器
    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = MemoryTypeIndex;

    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &OutImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(Device, OutImage, OutImageMemory, 0);
}

 VkImageView FVulkanUtility::CreateImageView(VkDevice Device,
                                               VkImage Image,
                                               VkFormat Format,
                                               VkImageAspectFlags AspectFlags,
                                               VkImageViewType ViewType,
                                               uint32_t MipLevelCount,
                                               uint32_t LayerCount
                                               ) {
    VkImageViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.image = Image;
    CreateInfo.viewType = ViewType;
    CreateInfo.format = Format;

    CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.subresourceRange.aspectMask = AspectFlags;
    CreateInfo.subresourceRange.baseMipLevel = 0;
    CreateInfo.subresourceRange.levelCount = MipLevelCount;
    CreateInfo.subresourceRange.baseArrayLayer = 0;
    CreateInfo.subresourceRange.layerCount = LayerCount;

    VkImageView ImageView;
    if (vkCreateImageView(Device, &CreateInfo, nullptr, &ImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
    }

    return ImageView;
}

//TODO 参数太多。 想办法放入FVulkanFrameBufferAttachment成员方法一些
 void FVulkanUtility::CreateFrameAttachment(FVulkanFrameBufferAttachment& OutAttachment,
                                               VkDevice                      Device,
                                               VkPhysicalDevice              Gpu,
                                               VkImageTiling                 Tiling,
                                               VkImageUsageFlags             Usage,
                                               VkMemoryPropertyFlags         MemoryProperty,
                                               VkImageAspectFlags            AspectFlags,
                                               VkImageViewType               ViewType,
                                               uint32_t                      ArrayLayers,
                                               uint32_t                      MipLevelCount)
 {
    //确保OutAttachment的格式已经有正确的值

    //创建图片
    CreateImage(Device,
                Gpu,
                OutAttachment.Width,
                OutAttachment.Height,
                OutAttachment.Format,
                ArrayLayers,
                Tiling,
                Usage,
                MemoryProperty,
                OutAttachment.Image,
                OutAttachment.Mem);

    //创建ImageView
    OutAttachment.View = CreateImageView(Device,
                                         OutAttachment.Image,
                                         OutAttachment.Format,
                                         AspectFlags,
                                         ViewType,
                                         MipLevelCount,
                                         ArrayLayers);
 }
