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
        //Cmd.AllocateCommandBuffer(Device2, CmdPool);
    }
}
