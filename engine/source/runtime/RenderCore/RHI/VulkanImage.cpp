//
// Created by zhibin.liu on 2024/2/1.
//

#include "VulkanImage.h"
#include "VulkanUtility.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <stdexcept>

void FVulkanImage::Destroy(VkDevice& Device)
{
//    vkDestroyBuffer(Device, RawBuffer, nullptr);
//    vkFreeMemory(Device, RawBufferMemory, nullptr);
}
void FVulkanImage::InnerCreateBuffer(VkDevice& Device,
                                      VkPhysicalDevice& Gpu,
                                      VkBufferUsageFlags Usage,
                                      VkMemoryPropertyFlags MemoryFlag)
{
//    VkBufferCreateInfo BufferInfo {};
//    BufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    BufferInfo.size        = BufferSize;
//    BufferInfo.usage       = Usage;                     // use as a vertex/staging/index buffer
//    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families
//    //创建缓冲区, 仅仅是声明了一个地址, 还没有实际匹配的显存
//    if (vkCreateBuffer(Device, &BufferInfo, nullptr, &RawBuffer) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to vkCreateBuffer");
//    }
//
//    //接下来为buffer申请实际的显存
//    //获取内存需求, 这些Buffer被创建出来的时候, 内存占用大小已经确定
//    VkMemoryRequirements MemRequirements;
//    vkGetBufferMemoryRequirements(Device, RawBuffer, &MemRequirements);
//
//    //开辟内存空间
//    //获取显卡的显存的属性. 包含显存类型(包含内存类型和对应的堆索引)和
//    //显存堆数据(包含堆大小和堆类型--一般都是device_local)
//    VkPhysicalDeviceMemoryProperties MemProperties;
//    vkGetPhysicalDeviceMemoryProperties(Gpu, &MemProperties);
//
//    //还创建了专门的VkDeviceMemory对象. 到这里猜测一下. buffer只是个指针, 真正的内存分配和拥有者还是个独立的对象
//    uint32_t MemoryTypeIndex = FVulkanUtility::FindMemoryType(MemProperties,
//                                                              MemRequirements.memoryTypeBits,
//                                                              MemoryFlag);
//
//    VkMemoryAllocateInfo AllocInfo{};
//    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    AllocInfo.allocationSize = MemRequirements.size;
//    AllocInfo.memoryTypeIndex = MemoryTypeIndex;
//
//    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &RawBufferMemory) != VK_SUCCESS)
//    {
//        throw std::runtime_error("vkAllocateMemory Error");
//    }
//
//    // 绑定缓冲区指针和实际的显存存
//    vkBindBufferMemory(Device, RawBuffer, RawBufferMemory, 0); // offset = 0
}