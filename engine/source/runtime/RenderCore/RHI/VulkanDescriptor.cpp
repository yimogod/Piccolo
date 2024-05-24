#include "VulkanDescriptor.h"
#include "VulkanDevice.h"
#include <stdexcept>

void FVulkanDescriptorLayout::SetBindingNum(uint32_t Num)
{
    RawBindings.resize(Num);
}
void FVulkanDescriptorLayout::SetBinding(uint32_t           BindingIndex,
                                         VkDescriptorType   DescriptorType,
                                         VkShaderStageFlags StageFlags)
{
    //我们默认让插槽索引和绑定索引一致
    VkDescriptorSetLayoutBinding Binding{};
    Binding.binding = BindingIndex;
    //就一个描述符, 如果是多个描述符如何赋值使用?
    //在文档一张图理解描述符集中, 就提到了描述符是多个的情况. 如下
    //mvp包含3个值.所以这里的描述符数量可以设置为3. 但要注意的是有两点
    //1. 是在设置下一个BindingIndex的时候, 不是+1, 而是+3了. 比如上一次binding.binging = 0,
    // 那下一个binding.binging = 3, 而不是1了
    // 当前为了概念简单和实现简单只允许一个槽位对应一个描述符
    // 当然如果, 比如传入的uniform是个数组. 这里的Descriptor的Cout就是数组长度.
    //2. 在Write的时候, 也有变量descriptorCount需要和这里设置的一样
    Binding.descriptorCount = 1;
    Binding.descriptorType = DescriptorType; //作用是uniform, 另外还有贴图
    Binding.stageFlags = StageFlags;
    Binding.pImmutableSamplers = nullptr;

    //没明白为何其他地方可以通过获取引用的方式赋值. 而这个方法就不行
    RawBindings[BindingIndex] = Binding;
}
void FVulkanDescriptorLayout::CreateLayout(FVulkanDevice& Device)
{
    VkDescriptorSetLayoutCreateInfo CreateInfo;
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.bindingCount = RawBindings.size();
    CreateInfo.pBindings = RawBindings.data();

    if (VK_SUCCESS != vkCreateDescriptorSetLayout(Device.GetDevice(),
                                                  &CreateInfo,
                                                  nullptr,
                                                  &RawDescriptorSetLayout))
    {
        throw std::runtime_error("create FVulkanDescriptorLayout Error");
    }
}

void FVulkanDescriptorWrite::SetWriteNum(uint32_t Num)
{
    Writes.resize(Num);
}
void FVulkanDescriptorWrite::SetBuffer(uint32_t WriteIndex, VkDescriptorBufferInfo* Buffer)
{
    //这里还没有赋值描述符集 dstSet. 会在UpdateDescriptorSets里面进行赋值. 减少参数传递为了
    auto& Write = Writes[WriteIndex];
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.pNext = nullptr;
    Write.dstBinding = WriteIndex;
    Write.dstArrayElement = 0;
    Write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    Write.descriptorCount = 1;
    Write.pBufferInfo = Buffer;
}
void FVulkanDescriptorWrite::SetImage(uint32_t WriteIndex, VkDescriptorType DescriptorType, VkDescriptorImageInfo* Buffer)
{
    //使用 write 将framebuffer的imageview和描述符集绑定起来
    auto& Write = Writes[WriteIndex];
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.pNext = nullptr;
    Write.dstBinding = WriteIndex;
    Write.dstArrayElement = 0;
    Write.descriptorType  = DescriptorType;
    Write.descriptorCount = 1;
    Write.pImageInfo = Buffer;
}

void FVulkanDescriptorWrite::UpdateDescriptorSets(FVulkanDevice& Device, FVulkanDescriptorSet& DescriptorSet)
{
    for (auto& Write : Writes)
    {
        Write.dstSet = DescriptorSet.GetVkDescriptorSet();
    }

    vkUpdateDescriptorSets(Device.GetDevice(),
                           Writes.size(),
                           Writes.data(),
                           0,
                           nullptr);

    BufferInfos.clear();
    ImageInfos.clear();
}

void FVulkanDescriptorSet::SetBindingNum(uint32_t Num)
{
    Layout.SetBindingNum(Num);
    Write.SetWriteNum(Num);
}

void FVulkanDescriptorSet::SetLayoutBinding(uint32_t           BindingIndex,
                                            VkDescriptorType   DescriptorType,
                                            VkShaderStageFlags StageFlags)
{
    Layout.SetBinding(BindingIndex, DescriptorType, StageFlags);
}

void FVulkanDescriptorSet::SetWriteBuffer(uint32_t WriteIndex, VkDescriptorBufferInfo* Buffer)
{
    Write.SetBuffer(WriteIndex, Buffer);
}

void FVulkanDescriptorSet::SetWriteImage(uint32_t WriteIndex, VkDescriptorImageInfo* Buffer)
{
    //同一槽位的binding和write的描述符类型是一样的
    //这个index要和frag的 input_attachment_index 值一致
    //比如 layout(input_attachment_index = 1, set = 0, binding = 1) uniform highp subpassInput in_ui_color;
    //这里 input_attachment_index = 1, 如果描述符布局中的描述符个数都是1的话, 也和binding的值一致
    auto DescriptorType = Layout.RawBindings[WriteIndex].descriptorType;
    Write.SetImage(WriteIndex, DescriptorType, Buffer);
}

void FVulkanDescriptorSet::CreateDescriptorLayout(FVulkanDevice& Device)
{
    //先创建布局
    Layout.CreateLayout(Device);
}

void FVulkanDescriptorSet::CreateDescriptorSet(FVulkanDevice& Device, FVulkanDescriptorPool& Pool)
{
    if (Layout.RawDescriptorSetLayout == VK_NULL_HANDLE)
    {
        Layout.CreateLayout(Device);
    }

    //接着创建描述符集
    VkDescriptorSetAllocateInfo Info;
    Info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    Info.pNext              = nullptr;
    Info.descriptorPool     = Pool.GetVkPool();
    Info.descriptorSetCount = 1;
    Info.pSetLayouts        = &Layout.RawDescriptorSetLayout;

    //创建描述符集
    if (VK_SUCCESS != vkAllocateDescriptorSets(Device.GetDevice(), &Info,&RawDescriptorSet))
    {
        throw std::runtime_error("allocate CreateDescriptorSet Error");
    }
}

void FVulkanDescriptorSet::UpdateDescriptorSets(FVulkanDevice& Device)
{
    Write.UpdateDescriptorSets(Device, *this);
}
