#pragma once

#include "vulkan/vulkan_core.h"
#include <vector>

class FVulkanDescriptorSet;

//描述符布局, 就是一对shader, 包含所有的vs ps所需的所有的uniform/texture的槽位信息
//简单的不太对的总结一下. 描述符集和Shader密切相关.描述的是shader的buffer的格式和槽位信息
//而pipeline布局其实是由描述符布局数组构成. 之前也可以认为pipeline和shader就是一一对应的
//简单点理解, 一个描述符就是shader用的数据的描述. Layout布局的是绑定, 也就是说描述符集布局的对象是描述符

//而附件虽然也指向同样的imageview, 但概念上更服务于subpass

//粗略理解. imageview作为shader的输入是描述符集, 及shader使用的资源. 作为shader的输出是附件!!!
//所以目前看到的代码附件都是在PS里面有用


//纹理   \
//采样器   \
//纹理       描述集布局1
//缓冲区   /           \
//缓冲区  /             \
//                       管线布局
//纹理    \             /
//纹理     \           /
//纹理      描述集布局2
//采样器  /
//采样器 /

//在shader中 layout()声明符有两类. 
// 1. layout(set = 0, binding = 0)表示的是描述符集绑定的资源
// 2. layout(location = 0)表示的vs/ps的输入和输出的数据
class FVulkanDescriptorLayout
{
    friend FVulkanDescriptorSet;
public:
    FVulkanDescriptorLayout() = default;

    //共需要多少个绑定或者槽位
    void SetBindingNum(uint32_t Num);

    //设置第BindingIndex的描述符数据
    void SetBinding(uint32_t BindingIndex, VkDescriptorType DescriptorType, VkShaderStageFlags StageFlags);

    //根据传入的bindling参数, 调用vulkan api创建 描述符布局
    void CreateLayout(VkDevice& Device);

private:
    //描述符集布局
    VkDescriptorSetLayout RawDescriptorSetLayout = VK_NULL_HANDLE;

    //描述符绑定布局槽点, 每个着色器可访问的资源都具有一个绑定序号. 资源有image, buffer, sampler, input attachment等等
    std::vector<VkDescriptorSetLayoutBinding> RawBindings;
};

//写入器, 指定对应的描述符集对应的插槽的Buffer资源(也可以是Image)
class FVulkanDescriptorWrite
{
    friend FVulkanDescriptorSet;
public:
    FVulkanDescriptorWrite() = default;

    //和布局的槽位个数一致
    void SetWriteNum(uint32_t Num);

    //设置第 WriteIndex 个槽点对应的buffer
    void SetBuffer(uint32_t WriteIndex, VkDescriptorBufferInfo* Buffer);

    //设置第 WriteIndex 个槽点对应的buffer
    void SetImage(uint32_t WriteIndex, VkDescriptorType DescriptorType, VkDescriptorImageInfo* Buffer);

    //根据Write的数据,更新描述符集
    void UpdateDescriptorSets(VkDevice Device, VkDescriptorSet DescriptorSet);

private:
    //描述符集对应的写入器数组
    std::vector<VkWriteDescriptorSet> Writes;

    //缓冲创建writer的中间数据. 否则会被提前释放
    uint32_t BufferIndex = 0;
    std::vector <VkDescriptorBufferInfo> BufferInfos;
    //缓冲创建writer的中间数据. 否则会被提前释放
    uint32_t ImageIndex = 0;
    std::vector <VkDescriptorImageInfo> ImageInfos;
};

//描述符集, 这里默认了一个描述符集对应一个布局, Set:Layout = 1:1
//封装了原生描述符集, 描述符布局和写入器
//提供了更新数据的接口
//shader通过描述符集来访问资源, 描述符集表示绑定到管线的资源的集合

//要访问描述符集附带的资源，描述符集必须要绑定到命令缓冲区
class FVulkanDescriptorSet
{
public:
    FVulkanDescriptorSet() = default;

    //获取描述符集对应的布局
    VkDescriptorSetLayout& GetLayout(){ return Layout.RawDescriptorSetLayout; }

    //获取原生描述符集
    VkDescriptorSet& GetDescriptorSet(){ return RawDescriptorSet; }

    //共需要多少个绑定或者槽位
    void SetBindingNum(uint32_t Num);

    //设置第BindingIndex的数据
    void SetLayoutBinding(uint32_t BindingIndex, VkDescriptorType DescriptorType, VkShaderStageFlags StageFlags);

    //设置第 WriteIndex 个槽点对应的buffer
    void SetWriteBuffer(uint32_t WriteIndex, VkDescriptorBufferInfo* Buffer);

    //设置第 WriteIndex 个槽点对应的buffer
    void SetWriteImage(uint32_t WriteIndex, VkDescriptorImageInfo* Buffer);

    //创建布局
    void CreateDescriptorLayout(VkDevice& Device);

    //调用api创建描述符集
    void CreateDescriptorSet(VkDevice& Device, VkDescriptorPool& Pool);

    //根据Write更新描述符集
    void UpdateDescriptorSets(VkDevice &Device);
private:
    //描述符集对应的布局
    FVulkanDescriptorLayout Layout;
    //描述符集对应的布局的写入器
    FVulkanDescriptorWrite Write;

    //描述符vulkan对象
    VkDescriptorSet RawDescriptorSet;
};

