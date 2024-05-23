#pragma once

#include "VulkanRHI.h"

// 某一个pass的vulkan renderpass渲染数据的集合. 对应了
// 一个(或3个)FrameBuffer
// 一个RenderPass
// 一套描述符集
// 一套Vulkan管线
class FVulkanPassPacket
{
public:
    FVulkanPassPacket() = default;

    //方便调用, 提供了原始vk的接口
    VkPipeline& GetVkPipeline(uint32_t Index) { return Pipelines[Index].GetPipeline(); }
    VkPipelineLayout& GetVkPipelineLayout(uint32_t Index) { return Pipelines[Index].GetLayout(); }
    VkDescriptorSet& GetVkDescriptorSet(uint32_t Index) { return DescriptorSets[Index].GetDescriptorSet(); }
    VkDescriptorSetLayout& GetVkDescriptorLayout(uint32_t Index) { return DescriptorSets[Index].GetLayout(); }

    //针对一些简易情况, 比如只有一个pipeline, 一个描述符集. 可以直接获取第一个值
    VkPipeline& GetVkPipeline() { return Pipelines[0].GetPipeline(); }
    VkPipelineLayout& GetVkPipelineLayout() { return Pipelines[0].GetLayout(); }
    VkDescriptorSet& GetVkDescriptorSet() { return DescriptorSets[0].GetDescriptorSet(); }
    VkDescriptorSetLayout& GetVkDescriptorLayout() { return DescriptorSets[0].GetLayout(); }


    //帧缓冲, 可能以1个也可能是3个(三缓冲)
    FVulkanFrameBuffer FrameBuffer;

    //当前frame buffer用到的渲染通道, 包含了
    //子通道
    //用到所有附件
    //子通道依赖
    FVulkanRenderPass RenderPass;

    // 1.和frame相关的
    // 2.本Pass用到的所有的描述符集.
    // 3.和Subpass的个数无关.
    //另外每个mesh也有自己的描述符集. 在绘制单个mesh, 而不是针对整个屏幕的操作时, 用的是单mesh的描述符集
    //会从这些所有的 描述符集子集来组装管线布局. 一个管线对应一个subpass
    std::vector<FVulkanDescriptorSet> DescriptorSets;

    //Pass用到的所有的管线, 可能是多个
    std::vector<FVulkanPipeline> Pipelines;

    //Pass用到的所有的附件. 多个
    std::vector<FVulkanAttachmentDescription> Attachments;
};
