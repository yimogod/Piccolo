#include "VulkanDevice.h"

void FVulkanDevice::CreateDevice()
{

}

void FVulkanDevice::CreateInstance()
{
//    // validation layer will be enabled in debug mode
//    if (m_enable_validation_Layers && !checkValidationLayerSupport())
//    {
//        LOG_ERROR("validation layers requested, but not available!");
//    }
//
//    m_vulkan_api_version = VK_API_VERSION_1_0;
//
//    // app info
//    VkApplicationInfo appInfo {};
//    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//    appInfo.pApplicationName   = "piccolo_renderer";
//    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//    appInfo.pEngineName        = "Piccolo";
//    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
//    appInfo.apiVersion         = m_vulkan_api_version;
//
//    // create info
//    VkInstanceCreateInfo instance_create_info {};
//    instance_create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//    instance_create_info.pApplicationInfo = &appInfo; // the appInfo is stored here
//
//    auto extensions                              = getRequiredExtensions();
//    instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
//    instance_create_info.ppEnabledExtensionNames = extensions.data();
//
//    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
//    if (m_enable_validation_Layers)
//    {
//        instance_create_info.enabledLayerCount   = static_cast<uint32_t>(m_validation_layers.size());
//        instance_create_info.ppEnabledLayerNames = m_validation_layers.data();
//
//        populateDebugMessengerCreateInfo(debugCreateInfo);
//        instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
//    }
//    else
//    {
//        instance_create_info.enabledLayerCount = 0;
//        instance_create_info.pNext             = nullptr;
//    }
//
//    // create m_vulkan_context._instance
//    if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS)
//    {
//        LOG_ERROR("vk create instance");
//    }
}
