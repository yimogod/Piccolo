#include "VulkanProxy.h"
void UVulkanProxy::Initialized()
{
    DepthFormat = FVulkanUtility::FindDepthFormat(Gpu);
}
