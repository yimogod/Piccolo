#include "VulkanProxy.h"
void UVulkanProxy::Initialized()
{
    Device2.SetDevice(Device);
    Device2.SetGPU(Gpu);

    DepthFormat = FVulkanUtility::FindDepthFormat(Gpu);
}
