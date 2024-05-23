#pragma once

#include "VulkanDescriptor.h"
#include "vulkan/vulkan_core.h"
#include <vector>

class FVulkanPipeline;

//这里的Shader概念其实是Shader组. 目前包含了顶点和像素shader
class FVulkanShader
{
public:
    FVulkanShader() = default;

    //根据顶点和像素shader的二进制, 创建shader对象
    void CreateShader(VkDevice Device, const std::vector<unsigned char>& VertCode, const std::vector<unsigned char>& FragCode);
    //根据顶点几何和像素的shader二进制, 创建shader对象
    void CreateShader(VkDevice Device, const std::vector<unsigned char>& VertCode, const std::vector<unsigned char>& GeomCode, const std::vector<unsigned char>& FragCode);

    //根据shader module创建ShaderStage数组
    void CreateStages(std::vector<VkPipelineShaderStageCreateInfo>& OutStages);

    //销毁ShaderModule
    void DestroyShader(VkDevice Device);
private:
    //创建单个shader
    void CreateShaderModule(VkDevice Device, const std::vector<unsigned char>& ShaderCode, VkShaderModule& OutShader);

    //创建shader应用的地方
    VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderModule& Shader, VkShaderStageFlagBits Stage);

    VkShaderModule VertShader = VK_NULL_HANDLE;
    VkShaderModule FragShader = VK_NULL_HANDLE;
    VkShaderModule GeomShader = VK_NULL_HANDLE;
};

class FVulkanPipelineState
{
    friend class FVulkanPipeline;
public:
    FVulkanPipelineState() = default;

    //设置一个默认的状态. 然后根据不同的情况对不同的state设置不同的值
    void SetupDefaultState();

    //设置顶点输入绑定
    void SetVertexBindingDescription(std::vector<VkVertexInputBindingDescription>& VertexBindings);
    //设置顶点输入属性
    void SetVertexAttributeDescription(std::vector<VkVertexInputAttributeDescription>& VertexAttributes);

    //设置视口的状态
    void SetViewportState(VkViewport& Viewport, VkRect2D& Scissor);

    //设置光栅化三角形正方向
    void SetRasterFrontFace(VkFrontFace Face);

    //添加颜色混合数据, 用于创建混合状态
    void SetColorBlendNum(uint32_t Num);
    void SetColorBlendAttachmentEnable(uint32_t Index, VkBool32 BlendEnable);
    void SetColorBlendFactor(uint32_t Index, VkBlendFactor SrcColorFactor, VkBlendFactor DstColorFactor, VkBlendOp ColorOp);
    void SetAlphaBlendFactor(uint32_t Index, VkBlendFactor SrcAlphaFactor, VkBlendFactor DstAlphaFactor, VkBlendOp AlphaOp);
    void SetupColorBlend();

    //设置深度状态
    void SetDepthState(VkBool32 bTestEnable, VkBool32 bWriteEnable, VkCompareOp CompareOP, VkBool32 bBoundTestEnable);
    void SetStencilState(VkBool32 bTestEnable);

    //禁用默认的动态状态
    void SetupNullDynamicState();

    void CopyVertInputState(VkPipelineVertexInputStateCreateInfo& InOutState);
    void CopyAssemblyState(VkPipelineInputAssemblyStateCreateInfo& InOutState);
    void CopyViewportState(VkPipelineViewportStateCreateInfo& InOutState);
    void CopyRasterState(VkPipelineRasterizationStateCreateInfo& InOutState);
    void CopySampleState(VkPipelineMultisampleStateCreateInfo& InOutState);
    void CopyColorBlendState(VkPipelineColorBlendStateCreateInfo& InOutState);
    void CopyDepthState(VkPipelineDepthStencilStateCreateInfo& InOutState);
    void CopyDynamicState(VkPipelineDynamicStateCreateInfo& InOutState);
private:
    // ----------- 创建管线需要的各种状态对象 -------------
    //顶点输入布局
    VkPipelineVertexInputStateCreateInfo VertInputState;

    //图元装配
    VkPipelineInputAssemblyStateCreateInfo AssembleState;

    //视口, 没有default值
    VkPipelineViewportStateCreateInfo ViewportState;

    //光栅化
    VkPipelineRasterizationStateCreateInfo RasterState;

    //多重采用
    VkPipelineMultisampleStateCreateInfo SampleState;

    //可能会有多个颜色混合, 所有在设置State前, 先填充了AttachmentState
    std::vector<VkPipelineColorBlendAttachmentState> ColorBlendData;
    VkPipelineColorBlendStateCreateInfo ColorBlendState;

    //深度状态
    VkPipelineDepthStencilStateCreateInfo DepthState;

    //动态状态, 视口和裁剪
    //图形管线很大，很复杂，也包含很多状态。在很多图形应用程序中，我们期望能够高频率地改变一些状态
    //将图形管线中的特定部分标记为动态的能力，这意味着它们可以直接使用命令缓冲区里的命令在运行时改变，而不是通过用不同的对象。
    //因为这会减少Vulkan优化的机会，或者会减少一部分状态，所以有必要精确地指定你想要将什么状态变为动态的
    std::vector<VkDynamicState> DynamicStateData;
    VkPipelineDynamicStateCreateInfo DynamicState;
};

class FVulkanPipeline
{
public:
    FVulkanPipeline() = default;

    VkPipelineLayout& GetLayout(){ return RawPipelineLayout; }
    VkPipeline& GetPipeline(){ return RawPipeline;}

    //根据传入的描述符的布局数组来创建管线布局, DescriptorSets是本管线(subpass/shader)用到的描述符集合. 而不是整个pass的所有的描述符集合
    void CreateLayout(VkDevice& Device, std::vector<FVulkanDescriptorSet>& DescriptorSets);
    void CreateLayout(VkDevice& Device, std::vector<VkDescriptorSetLayout>& DescriptorLayouts);

    void CreatePipeline(VkDevice& Device, VkRenderPass& RenderPass, uint32_t SubpassIndex,
                        std::vector<VkPipelineShaderStageCreateInfo>& ShaderStages, FVulkanPipelineState& State);

private:
    //原生管线对象
    VkPipeline RawPipeline = {};
    
    //原生管线布局
    //Pipeline通过PipelineLayout, 把描述符集和Shader关联了起来
    VkPipelineLayout RawPipelineLayout = {};
};


