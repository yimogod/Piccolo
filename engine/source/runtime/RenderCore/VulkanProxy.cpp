#include "VulkanProxy.h"
void UVulkanProxy::Initialized()
{
    Device2.SetDevice(Device);
    Device2.SetGPU(Gpu);
    DescriptorPool2.SetPool(DescriptorPool);

    DepthFormat = FVulkanUtility::FindDepthFormat(Gpu);
}
