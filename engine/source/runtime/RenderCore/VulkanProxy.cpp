#include "VulkanProxy.h"
void UVulkanProxy::Initialized()
{
    Device2.SetDevice(Device);
    Device2.SetGPU(Gpu);
    DescriptorPool2.SetPool(DescriptorPool);
    CmdPool.SetPool(CmdPool2);

    DepthFormat = FVulkanUtility::FindDepthFormat(Gpu);

    //分配CommandBuffer
    CmdBuffers.resize(SMaxFramesInFlight);
    for(auto& Cmd : CmdBuffers)
    {
        Cmd.AllocateCommandBuffer(Device2, CmdPool);
    }
}

void UVulkanProxy::PrepareContext()
{
    CurrCmdBuffer = CmdBuffers[CurrFrameIndex];
}

void UVulkanProxy::PushEvent(std::string Name, const float* Color)
{
    if (m_enable_debug_utils_label)
    {
        VkDebugUtilsLabelEXT label_info;
        label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label_info.pNext = nullptr;
        label_info.pLabelName = Name.c_str();
        for (int i = 0; i < 4; ++i)
            label_info.color[i] = Color[i];

        //vkCmdBeginDebugUtilsLabelEXT(CurrCmdBuffer.GetVkCommandBuffer(), &label_info);
    }
}