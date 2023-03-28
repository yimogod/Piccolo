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
class FVulkanBuffer
{
public:
    FVulkanBuffer() = default;
    //用于兼容现有引擎使用方式, 在外部已经创建了Buffer
    explicit FVulkanBuffer(VkBuffer& InBuffer) { RawBuffer = InBuffer; }

    VkBuffer& GetVkBuffer() { return RawBuffer; }
    uint64_t GetBufferSize() { return BufferSize; }

    //使用自己的buffer对象, 创建描述符buffer对象
    VkDescriptorBufferInfo CreateDescriptorBufferInfo(uint32_t Range);

    void Destroy(VkDevice& Device);
protected:
    void InnerCreateBuffer(VkDevice& Device, VkPhysicalDevice& Gpu, VkBufferUsageFlags Usage,
                           VkMemoryPropertyFlags MemoryProperty);

    VkBuffer       RawBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory RawBufferMemory = VK_NULL_HANDLE;
    //缓存当前buff的大小
    uint64_t BufferSize = 0;
};

class FVulkanStagingBuffer : public FVulkanBuffer
{
public:
    FVulkanStagingBuffer() = default;

    //接触内存和显存的映射.
    void UnmapMemory(VkDevice& Device);


    //映射显存Buffer的内存地址. 给这个Data赋值, 就相当于给Buffer赋值了
    //TODO 移到protected里
    void* Data = nullptr;
protected:

};

//用于Storage的Staging Buffer
class FVulkanStagingBuffer_Storage : public FVulkanStagingBuffer
{
public:
    //存储*Data的地址偏移
    struct BufferBlock
    {
        //分块数据的开始地址. 赋值完毕后不会再改变
        uint32_t Begin;
        //并不是真正意义的End. 应该叫做
        uint32_t End;
        uint32_t Size;
    };

    FVulkanStagingBuffer_Storage() = default;

    //获取分块数据当前的End值
    uint32_t GetBlockEnd(uint32_t BlockIndex) { return Blocks[BlockIndex].End; }

    //根据传入的数据初始化Staging Buffer
    //BlockNum是在用的时候, 分了多少块. 比如3缓冲, 就是三块.
    //InMinOffsetAlign查找数据块时的最小单位
    void Initialize(uint64_t TotalBufferSize, uint32_t BlockNum, uint32_t InMinOffsetAlign);

    //根据参数创建实际的Buffer
    void CreateBuffer(VkDevice& Device, VkPhysicalDevice& Gpu);

    //清理Block的数据, 即把End移到Begin
    void ClearBlockData(uint32_t BlockIndex);

    //将Value值赋予Block的GPU的Buffer上
    //EndMoveOffset在赋值后, 会把自己的位置向右偏移. 作为End的新位置.
    //在实际使用过程中, EndMoveOffset的值可以和T的size相同, 也可以不同
    //1. 将现有End地址对齐到MinOffsetAlignment上
    //2. 在此新的对齐的位置, 赋值Buffer
    //3. 然后偏移 EndMoveOffset, 重新指定为End的地址
    //4. 返回的是第一步End地址对齐后的偏移和以及此偏移强转的对象
    template<typename T>
    T& GetObjectAtEndAddress(uint32_t BlockIndex, uint32_t EndMoveOffset, uint32_t& OutAlignOffset);
private:
    //显存对齐的最小数值, 对应gpu的minStorageBufferOffsetAlignment
    uint32_t MinOffsetAlignment {256};

    //显存的最大Range, 对应gpu的maxStorageBufferRange. 默认值128M
    uint32_t MaxRange {1 << 27};

    //整个内存分块的
    std::vector<BufferBlock> Blocks;
};

//模板写在cpp中就链接错误, 搜索引擎提示
// 1. 写在.h中即可解决
// 2. 在使用的地方同时include ".cpp"也可解决
template<typename T>
T& FVulkanStagingBuffer_Storage::GetObjectAtEndAddress(uint32_t BlockIndex, uint32_t EndMoveOffset, uint32_t& OutAlignOffset)
{
    //计算End所在的片区(共三大片)且对齐的显存地址偏移
    OutAlignOffset = FMathUtility::RoundUp(Blocks[BlockIndex].End, MinOffsetAlignment);

    //重新计算End的新位置, 还没往后看. 预测这个storage buffer用来给uniform同步值的
    Blocks[BlockIndex].End = OutAlignOffset + EndMoveOffset;
    assert(Blocks[BlockIndex].End <= (Blocks[BlockIndex].Begin + Blocks[BlockIndex].Size));

    //这相当于给显存映射的内存赋值了
    T& Temp = (*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(Data) + OutAlignOffset));
    return Temp;
}