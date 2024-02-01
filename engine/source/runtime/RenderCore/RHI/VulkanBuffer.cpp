#include "VulkanBuffer.h"
#include "VulkanUtility.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <stdexcept>

VkDescriptorBufferInfo FVulkanBuffer::CreateDescriptorBufferInfo(uint32_t Range) 
{
    VkDescriptorBufferInfo Info = {};
    // this offset plus dynamic_offset should not be greater than the size of the buffer
    Info.offset = 0;
    // the range means the size actually used by the shader per draw call
    Info.range = Range;
    Info.buffer = RawBuffer;

    //assert(mesh_directional_light_shadow_perframe_storage_buffer_info.range <
    //       m_global_render_resource->_storage_buffer._max_storage_buffer_range);

    return Info;
}

void FVulkanBuffer::Destroy(VkDevice& Device)
{
    vkDestroyBuffer(Device, RawBuffer, nullptr);
    vkFreeMemory(Device, RawBufferMemory, nullptr);
}
void FVulkanBuffer::InnerCreateBuffer(VkDevice& Device,
                                      VkPhysicalDevice& Gpu,
                                      VkBufferUsageFlags Usage,
                                      VkMemoryPropertyFlags MemoryFlag)
{
    VkBufferCreateInfo BufferInfo {};
    BufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size        = BufferSize;
    BufferInfo.usage       = Usage;                     // use as a vertex/staging/index buffer
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing among queue families
    //创建缓冲区, 仅仅是声明了一个地址, 还没有实际匹配的显存
    if (vkCreateBuffer(Device, &BufferInfo, nullptr, &RawBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to vkCreateBuffer");
    }

    //接下来为buffer申请实际的显存
    //获取内存需求, 这些Buffer被创建出来的时候, 内存占用大小已经确定
    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(Device, RawBuffer, &MemRequirements);

    //开辟内存空间
    //获取显卡的显存的属性. 包含显存类型(包含内存类型和对应的堆索引)和
    //显存堆数据(包含堆大小和堆类型--一般都是device_local)
    VkPhysicalDeviceMemoryProperties MemProperties;
    vkGetPhysicalDeviceMemoryProperties(Gpu, &MemProperties);

    //还创建了专门的VkDeviceMemory对象. 到这里猜测一下. buffer只是个指针, 真正的内存分配和拥有者还是个独立的对象
    uint32_t MemoryTypeIndex = FVulkanUtility::FindMemoryType(MemProperties,
                                                                 MemRequirements.memoryTypeBits,
                                                                 MemoryFlag);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = MemoryTypeIndex;

    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &RawBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateMemory Error");
    }

    // 绑定缓冲区指针和实际的显存存
    vkBindBufferMemory(Device, RawBuffer, RawBufferMemory, 0); // offset = 0
}
void FVulkanStagingBuffer::UnmapMemory(VkDevice& Device)
{
    vkUnmapMemory(Device, RawBufferMemory);
}
void FVulkanStagingBuffer_Storage::Initialize(uint64_t TotalBufferSize, uint32_t BlockNum, uint32_t InMinOffsetAlign)
{
    BufferSize = TotalBufferSize;
    Blocks.resize(BlockNum);
    MinOffsetAlignment = InMinOffsetAlign;

    //分配三块内存
    for (uint32_t i = 0; i < BlockNum; ++i)
    {
        auto& Block = Blocks[i];
        Block.Begin = (BufferSize * i) / BlockNum;
        //有病吧. 这么复杂, 你直接 global_storage_buffer_size/3 不就行了
        //可能是因为不是3的整倍数. 所有为了避免错误. 使用了加减操作. 保证了和为1
        Block.Size = (BufferSize * (i + 1)) / BlockNum - (BufferSize * i) / BlockNum;
        Block.End = Block.Begin;
    }
}
void FVulkanStagingBuffer_Storage::CreateBuffer(VkDevice& Device, VkPhysicalDevice& Gpu)
{
    //申请128m的暂存数据缓冲区, cpu可访问, 这个usage用于storage, 而不是transfer
    InnerCreateBuffer(Device,
                      Gpu,
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                      );

    vkMapMemory(Device, RawBufferMemory, 0, BufferSize, 0, &Data);

    //设置store buffer的对齐数值
    VkPhysicalDeviceProperties GpuProperties;
    vkGetPhysicalDeviceProperties(Gpu, &GpuProperties);
    MinOffsetAlignment = static_cast<uint32_t>(GpuProperties.limits.minStorageBufferOffsetAlignment);
    MaxRange = GpuProperties.limits.maxStorageBufferRange;
}
void FVulkanStagingBuffer_Storage::ClearBlockData(uint32_t BlockIndex)
{
    Blocks[BlockIndex].End = Blocks[BlockIndex].Begin;
}
