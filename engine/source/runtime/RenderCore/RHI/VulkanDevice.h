#pragma once

#include "vulkan/vulkan_core.h"

class FVulkanImage;

//只有Device类似的统筹类, 参数里面才能包含原生的vk对象.
//其他vulkan对象只能是Get方法包括vk对象
class FVulkanDevice
{
public:
    FVulkanDevice() = default;

    //TODO 兼容旧引擎
    void SetDeivce(VkDevice& InDevice){Device = InDevice;}
    void SetGPU(VkPhysicalDevice& InGPU){GPU = InGPU;}
    void SetInstance(VkInstance& InInstance){Instance = InInstance;}

    VkDevice& GetDevice() { return Device; }
    VkPhysicalDevice& GetGPU() { return GPU; }

    void CreateDevice();

    //创建图片
    VkResult CreateImageView(VkImageViewCreateInfo* pCreateInfo, VkImageView* pView);
    //销毁图片视图
    void DestroyImageView(VkImageView& View);

protected:
    void CreateInstance();

private:
    VkDevice Device;
    VkPhysicalDevice GPU;
    VkInstance Instance;
};