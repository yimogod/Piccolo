#include "VulkanPipeline.h"
#include <stdexcept>

void FVulkanShader::CreateShader(VkDevice                          Device,
                                 const std::vector<unsigned char>& VertCode,
                                 const std::vector<unsigned char>& FragCode)
{
    CreateShaderModule(Device, VertCode, VertShader);
    CreateShaderModule(Device, FragCode, FragShader);
}
void FVulkanShader::CreateShader(VkDevice                          Device,
                                 const std::vector<unsigned char>& VertCode,
                                 const std::vector<unsigned char>& GeomCode,
                                 const std::vector<unsigned char>& FragCode)
{
    CreateShaderModule(Device, VertCode, VertShader);
    CreateShaderModule(Device, GeomCode, GeomShader);
    CreateShaderModule(Device, FragCode, FragShader);
}
void FVulkanShader::CreateStages(std::vector<VkPipelineShaderStageCreateInfo>& OutStages)
{
    //计算用到了那些shader
    uint32_t Size = 2;
    if (GeomShader != VK_NULL_HANDLE)
    {
        Size++;
    }


    //同样规则创建stage
    OutStages.resize(Size);
    uint32_t i = 0;

    //vert stage
    OutStages[i] = CreateShaderStage(VertShader, VK_SHADER_STAGE_VERTEX_BIT);
    i++;

    //geom stage
    if (GeomShader != VK_NULL_HANDLE)
    {
        OutStages[1] = CreateShaderStage(GeomShader, VK_SHADER_STAGE_GEOMETRY_BIT);
        i++;
    }

    //pixel stage
    OutStages[i] = CreateShaderStage(FragShader, VK_SHADER_STAGE_FRAGMENT_BIT);
}
void FVulkanShader::CreateShaderModule(VkDevice                          Device,
                                       const std::vector<unsigned char>& ShaderCode,
                                       VkShaderModule&                   OutShader)
{
    VkShaderModuleCreateInfo CreateInfo {};
    CreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = ShaderCode.size();
    CreateInfo.pCode    = reinterpret_cast<const uint32_t*>(ShaderCode.data());

    if (vkCreateShaderModule(Device, &CreateInfo, nullptr, &OutShader) != VK_SUCCESS)
    {
        throw std::runtime_error("CreateShaderModule Error");
    }
}

VkPipelineShaderStageCreateInfo FVulkanShader::CreateShaderStage(VkShaderModule& Shader, VkShaderStageFlagBits Stage)
{
    VkPipelineShaderStageCreateInfo CreateInfo {};
    CreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    CreateInfo.stage  = Stage;
    CreateInfo.module = Shader;
    CreateInfo.pName  = "main";
    return CreateInfo;
}
void FVulkanShader::DestroyShader(VkDevice Device)
{
    vkDestroyShaderModule(Device, VertShader, nullptr);
    vkDestroyShaderModule(Device, FragShader, nullptr);

    if(GeomShader != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(Device, GeomShader, nullptr);
    }
}
void FVulkanPipelineState::SetupDefaultState()
{
    //没有顶点输入数据
    VertInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertInputState.vertexBindingDescriptionCount = 0;
    VertInputState.pVertexBindingDescriptions    = nullptr;
    VertInputState.vertexAttributeDescriptionCount = 0;
    VertInputState.pVertexAttributeDescriptions  = nullptr;

    AssembleState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    AssembleState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //拓扑是三角形列表
    AssembleState.primitiveRestartEnable = VK_FALSE; //关闭图元重启

    //光栅化
    RasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterState.depthClampEnable        = VK_FALSE; //关闭深度截取
    RasterState.rasterizerDiscardEnable = VK_FALSE; //启动光栅化
    RasterState.polygonMode             = VK_POLYGON_MODE_FILL; //绘制模式填充
    RasterState.lineWidth               = 1.0f; //线框模式下的线宽度
    RasterState.cullMode                = VK_CULL_MODE_BACK_BIT; //背面裁剪
    //RasterState.cullMode = VK_CULL_MODE_NONE; //不做背面裁剪
    RasterState.frontFace               = VK_FRONT_FACE_CLOCKWISE; ///顺时针为正方向
    RasterState.depthBiasEnable         = VK_FALSE; //关闭深度偏移
    RasterState.depthBiasConstantFactor = 0.0f;
    RasterState.depthBiasClamp          = 0.0f;
    RasterState.depthBiasSlopeFactor    = 0.0f;

    //多重采用
    SampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    SampleState.sampleShadingEnable  = VK_FALSE; //关闭采用
    SampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //颜色混合
    ColorBlendState.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendState.logicOpEnable = VK_FALSE;
    ColorBlendState.logicOp       = VK_LOGIC_OP_COPY;
    ColorBlendState.attachmentCount = 0;
    ColorBlendState.pAttachments = nullptr;
    ColorBlendState.blendConstants[0] = 0.0f;
    ColorBlendState.blendConstants[1] = 0.0f;
    ColorBlendState.blendConstants[2] = 0.0f;
    ColorBlendState.blendConstants[3] = 0.0f;

    //深度模板
    DepthState.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthState.depthTestEnable  = VK_FALSE; //关闭深度测试
    DepthState.depthWriteEnable = VK_FALSE; //关闭深度可写
    DepthState.depthCompareOp   = VK_COMPARE_OP_ALWAYS;
    DepthState.depthBoundsTestEnable = VK_FALSE;
    DepthState.stencilTestEnable     = VK_FALSE; //关闭模板测试

    //动态状态指定
    //动态状态指定
    DynamicStateData.resize(2);
    DynamicStateData[0] = VK_DYNAMIC_STATE_VIEWPORT;
    DynamicStateData[1] = VK_DYNAMIC_STATE_SCISSOR;
    DynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.dynamicStateCount = DynamicStateData.size();
    DynamicState.pDynamicStates    = DynamicStateData.data();
}
void FVulkanPipelineState::SetVertexBindingDescription(std::vector<VkVertexInputBindingDescription>& VertexBindings)
{
    VertInputState.vertexBindingDescriptionCount   = VertexBindings.size();
    VertInputState.pVertexBindingDescriptions      = VertexBindings.data();
}
void FVulkanPipelineState::SetVertexAttributeDescription(std::vector<VkVertexInputAttributeDescription>& VertexAttributes)
{
    VertInputState.vertexAttributeDescriptionCount = VertexAttributes.size();
    VertInputState.pVertexAttributeDescriptions    = VertexAttributes.data();
}
void FVulkanPipelineState::SetViewportState(VkViewport& Viewport, VkRect2D& Scissor)
{
    ViewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports    = &Viewport;
    ViewportState.scissorCount  = 1;
    ViewportState.pScissors     = &Scissor;
}
void FVulkanPipelineState::SetRasterFrontFace(VkFrontFace Face)
{
    RasterState.frontFace = Face;
}
void FVulkanPipelineState::SetColorBlendNum(uint32_t Num)
{
    ColorBlendData.resize(Num);
    //设置ColorWriteMask默认值
    for (auto& Attachment : ColorBlendData)
    {
        Attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }
}
void FVulkanPipelineState::SetColorBlendAttachmentEnable(uint32_t Index, VkBool32 BlendEnable)
{
    ColorBlendData[Index].blendEnable = BlendEnable;
}
void FVulkanPipelineState::SetColorBlendFactor(uint32_t      Index,
                                               VkBlendFactor SrcColorFactor,
                                               VkBlendFactor DstColorFactor,
                                               VkBlendOp     ColorOp)
{
    auto& Attachment = ColorBlendData[Index];
    Attachment.srcColorBlendFactor = SrcColorFactor;
    Attachment.dstColorBlendFactor = DstColorFactor;
    Attachment.colorBlendOp        = ColorOp;
}
void FVulkanPipelineState::SetAlphaBlendFactor(uint32_t      Index,
                                               VkBlendFactor SrcAlphaFactor,
                                               VkBlendFactor DstAlphaFactor,
                                               VkBlendOp     AlphaOp)
{
    auto& Attachment = ColorBlendData[Index];
    Attachment.srcAlphaBlendFactor = SrcAlphaFactor;
    Attachment.dstAlphaBlendFactor = DstAlphaFactor;
    Attachment.alphaBlendOp        = AlphaOp;
}
void FVulkanPipelineState::SetupColorBlend()
{
    ColorBlendState.logicOp       = VK_LOGIC_OP_COPY;
    ColorBlendState.attachmentCount = ColorBlendData.size();
    ColorBlendState.pAttachments      = ColorBlendData.data();
}
void FVulkanPipelineState::SetDepthState(VkBool32 bTestEnable, VkBool32 bWriteEnable, VkCompareOp CompareOP)
{
    DepthState.depthTestEnable = bTestEnable;
    DepthState.depthWriteEnable = bWriteEnable;
    DepthState.depthCompareOp = CompareOP;
}
void FVulkanPipelineState::SetupNullDynamicState()
{
    DynamicState.dynamicStateCount = 0;
    DynamicState.pDynamicStates    = nullptr;
}
void FVulkanPipelineState::CopyVertInputState(VkPipelineVertexInputStateCreateInfo& InOutState)
{
    InOutState.sType = VertInputState.sType;
    InOutState.vertexBindingDescriptionCount = VertInputState.vertexBindingDescriptionCount;
    InOutState.pVertexBindingDescriptions = VertInputState.pVertexBindingDescriptions;
    InOutState.vertexAttributeDescriptionCount = VertInputState.vertexAttributeDescriptionCount;
    InOutState.pVertexAttributeDescriptions = VertInputState.pVertexAttributeDescriptions;
}
void FVulkanPipelineState::CopyAssemblyState(VkPipelineInputAssemblyStateCreateInfo& InOutState)
{
    InOutState.sType = AssembleState.sType;
    InOutState.topology = AssembleState.topology;
    InOutState.primitiveRestartEnable = AssembleState.primitiveRestartEnable;
}
void FVulkanPipelineState::CopyViewportState(VkPipelineViewportStateCreateInfo& InOutState)
{
    InOutState.sType = ViewportState.sType;
    InOutState.viewportCount = ViewportState.viewportCount;
    InOutState.pViewports = ViewportState.pViewports;
    InOutState.scissorCount = ViewportState.scissorCount;
    InOutState.pScissors = ViewportState.pScissors;
}
void FVulkanPipelineState::CopyRasterState(VkPipelineRasterizationStateCreateInfo& InOutState)
{
    InOutState.sType = RasterState.sType;
    InOutState.depthClampEnable=RasterState.depthClampEnable;
    InOutState.rasterizerDiscardEnable = RasterState.rasterizerDiscardEnable;
    InOutState.polygonMode = RasterState.polygonMode;
    InOutState.lineWidth = RasterState.lineWidth;
    InOutState.cullMode = RasterState.cullMode;
    InOutState.frontFace = RasterState.frontFace;
    InOutState.depthBiasEnable = RasterState.depthBiasEnable;
    InOutState.depthBiasConstantFactor = RasterState.depthBiasConstantFactor;
    InOutState.depthBiasClamp = RasterState.depthBiasClamp;
    InOutState.depthBiasSlopeFactor = RasterState.depthBiasSlopeFactor;
}
void FVulkanPipelineState::CopySampleState(VkPipelineMultisampleStateCreateInfo& InOutState)
{
    InOutState.sType = SampleState.sType;
    InOutState.sampleShadingEnable = SampleState.sampleShadingEnable;
    InOutState.rasterizationSamples = SampleState.rasterizationSamples;
}
void FVulkanPipelineState::CopyColorBlendState(VkPipelineColorBlendStateCreateInfo& InOutState)
{
    InOutState.sType = ColorBlendState.sType;
    InOutState.logicOpEnable = ColorBlendState.logicOpEnable;
    InOutState.logicOp = ColorBlendState.logicOp;
    InOutState.attachmentCount = ColorBlendState.attachmentCount;
    InOutState.pAttachments = ColorBlendState.pAttachments;
    InOutState.blendConstants[0] = ColorBlendState.blendConstants[0];
    InOutState.blendConstants[1] = ColorBlendState.blendConstants[1];
    InOutState.blendConstants[2] = ColorBlendState.blendConstants[2];
    InOutState.blendConstants[3] = ColorBlendState.blendConstants[3];
}
void FVulkanPipelineState::CopyDepthState(VkPipelineDepthStencilStateCreateInfo& InOutState)
{
    InOutState.sType = DepthState.sType;
    InOutState.depthTestEnable = DepthState.depthTestEnable;
    InOutState.depthWriteEnable = DepthState.depthWriteEnable;
    InOutState.depthCompareOp = DepthState.depthCompareOp;
    InOutState.depthBoundsTestEnable = DepthState.depthBoundsTestEnable;
    InOutState.stencilTestEnable = DepthState.stencilTestEnable;
}
void FVulkanPipelineState::CopyDynamicState(VkPipelineDynamicStateCreateInfo& InOutState)
{
    InOutState.sType = DynamicState.sType;
    InOutState.dynamicStateCount = DynamicState.dynamicStateCount;
    InOutState.pDynamicStates = DynamicState.pDynamicStates;
}
void FVulkanPipeline::CreateLayout(VkDevice& Device, std::vector<FVulkanDescriptorSet>& DescriptorSets)
{
    std::vector<VkDescriptorSetLayout> DescriptorLayouts;
    for (auto& Set : DescriptorSets)
    {
        DescriptorLayouts.push_back(Set.GetLayout());
    }
    CreateLayout(Device, DescriptorLayouts);
}
void FVulkanPipeline::CreateLayout(VkDevice& Device, std::vector<VkDescriptorSetLayout>& DescriptorLayouts)
{
    //创建管线布局, 及这个Pipeline用到了多少个desc set.
    //于是有了layout(set = 1, binding = 2)
    VkPipelineLayoutCreateInfo CreateInfo {};
    CreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    CreateInfo.setLayoutCount = DescriptorLayouts.size();
    CreateInfo.pSetLayouts    = DescriptorLayouts.data();

    if (vkCreatePipelineLayout(Device,
                               &CreateInfo,
                               nullptr,
                               &RawPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("CreatePipelineLayout Error");
    }
}
void FVulkanPipeline::CreatePipeline(VkDevice& Device, VkRenderPass& RenderPass, uint32_t SubpassIndex,
                                     std::vector<VkPipelineShaderStageCreateInfo>& ShaderStages, FVulkanPipelineState& State)
{
    // 拷贝变量到本地. 可能跟vulkan的机制有关. 只有本地局部变量才能正确创建piplineline对象
    VkPipelineVertexInputStateCreateInfo VertInputState = {};
    State.CopyVertInputState(VertInputState);

    VkPipelineInputAssemblyStateCreateInfo AssemblyState {};
    State.CopyAssemblyState(AssemblyState);

    VkPipelineViewportStateCreateInfo ViewportState {};
    State.CopyViewportState(ViewportState);

    VkPipelineRasterizationStateCreateInfo tRasterState {};
    State.CopyRasterState(tRasterState);

    VkPipelineMultisampleStateCreateInfo tSampleState = {};
    State.CopySampleState(tSampleState);

    VkPipelineColorBlendStateCreateInfo tColorBlendState {};
    State.CopyColorBlendState(tColorBlendState);

    VkPipelineDepthStencilStateCreateInfo tDepthState {};
    State.CopyDepthState(tDepthState);

    VkPipelineDynamicStateCreateInfo tDynamicState {};
    State.CopyDynamicState(tDynamicState);

    VkGraphicsPipelineCreateInfo PipelineInfo {};
    PipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount          = ShaderStages.size();
    PipelineInfo.pStages             = ShaderStages.data();
    PipelineInfo.pVertexInputState   = &VertInputState;
    PipelineInfo.pInputAssemblyState = &AssemblyState;
    PipelineInfo.pViewportState      = &ViewportState;
    PipelineInfo.pRasterizationState = &tRasterState;
    PipelineInfo.pMultisampleState   = &tSampleState;
    PipelineInfo.pColorBlendState    = &tColorBlendState;
    PipelineInfo.pDepthStencilState  = &tDepthState;
    PipelineInfo.layout              = RawPipelineLayout;
    PipelineInfo.renderPass          = RenderPass;
    PipelineInfo.subpass             = SubpassIndex;
    PipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    PipelineInfo.pDynamicState       = &tDynamicState;

    if (vkCreateGraphicsPipelines(Device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &PipelineInfo,
                                  nullptr,
                                  &RawPipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("create deferred lighting graphics pipeline");
    }
}

