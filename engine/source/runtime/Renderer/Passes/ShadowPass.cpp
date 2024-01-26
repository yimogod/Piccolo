#include "ShadowPass.h"

#include "runtime/function/render/render_helper.h"
#include "runtime/function/render/render_mesh.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/render_common.h"

#include <mesh_directional_light_shadow_frag.h>
#include <mesh_directional_light_shadow_vert.h>

#include <stdexcept>

namespace Piccolo
{
    void UShadowPass::initialize(const FRenderPassInitInfo* init_info)
    {
        URenderPass::initialize(nullptr);

    	Proxy.FrameBuffer.Width  = s_directional_light_shadow_map_dimension;
        Proxy.FrameBuffer.Height = s_directional_light_shadow_map_dimension;

        //根据之前的经验, 这几个创建是由顺序依赖的
        setupAttachments();
        setupRenderPass();
        setupFramebuffer();

        //shader相关, 和后面的渲染管线有联系
        setupDescriptorSetLayout();
    }
    void UShadowPass::postInitialize()
    {
        setupPipelines();
        setupDescriptorSet();
    }
    void UShadowPass::preparePassData(std::shared_ptr<RenderResourceBase> render_resource)
    {
        const RenderResource* vulkan_resource = static_cast<const RenderResource*>(render_resource.get());
        if (vulkan_resource)
        {
            m_mesh_directional_light_shadow_perframe_storage_buffer_object =
                vulkan_resource->m_mesh_directional_light_shadow_perframe_storage_buffer_object;
        }
    }
    void UShadowPass::draw() { drawModel(); }

    //创建帧缓存需要的所有附件. 针对本Pass, 用到了2个附件.
    //一个是颜色, 一个是深度
    void UShadowPass::setupAttachments()
    {
        // 俩附件 颜色和深度

        //颜色格式32位R
        FVulkanFrameBufferAttachment Attachment_0;
        Attachment_0.Format = VK_FORMAT_R32_SFLOAT;
        Attachment_0.Width = Attachment_0.Height = Proxy.FrameBuffer.Height; //宽高一样
        FVulkanRHIUtility::CreateFrameAttachment(
            Attachment_0,
            m_rhi->m_device,
            m_rhi->m_physical_device,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, //用于采样和Ps输出--即shader的输入输出
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,                 //仅显卡可访问
            VK_IMAGE_ASPECT_COLOR_BIT                              // Color Bit
        );
        Proxy.FrameBuffer.AddFrameAttachment(Attachment_0);

        // depth
        FVulkanFrameBufferAttachment Attachment_1;
        Attachment_1.Format = (VkFormat)m_rhi->getDepthImageInfo().depth_image_format;
        Attachment_1.Width = Attachment_1.Height = Proxy.FrameBuffer.Height;
        FVulkanRHIUtility::CreateFrameAttachment(
            Attachment_1,
            m_rhi->m_device,
            m_rhi->m_physical_device,
            VK_IMAGE_TILING_OPTIMAL,
            //用于深度附件和交换格式的中间buffer
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //仅显卡可访问
            VK_IMAGE_ASPECT_DEPTH_BIT            // Depth Bit
        );
        Proxy.FrameBuffer.AddFrameAttachment(Attachment_1);


        ////////////////////////////// TODO 兼容代码  ///////////////////////
        // color and depth
        m_framebuffer.attachments.resize(2);

        // color
        FVulkanFrameBufferAttachment Att1   = Proxy.FrameBuffer.GetAttachment(0);
        m_framebuffer.attachments[0].format = (RHIFormat)Att1.Format;
        
        auto image = new VulkanImage();
        image->setResource(Att1.Image);
        m_framebuffer.attachments[0].image = image;

        auto image_view = new VulkanImageView();
        image_view->setResource(Att1.View);
        m_framebuffer.attachments[0].view  = image_view;

        auto memory = new VulkanDeviceMemory();
        memory->setResource(Att1.Mem);
        m_framebuffer.attachments[0].mem = memory;


        // depth
        FVulkanFrameBufferAttachment Att2   = Proxy.FrameBuffer.GetAttachment(1);
        m_framebuffer.attachments[1].format = (RHIFormat)Att2.Format;

        auto image2 = new VulkanImage();
        image2->setResource(Att2.Image);
        m_framebuffer.attachments[1].image = image2;

        auto image_view2 = new VulkanImageView();
        image_view2->setResource(Att2.View);
        m_framebuffer.attachments[1].view = image_view2;

        auto memory2 = new VulkanDeviceMemory();
        memory->setResource(Att2.Mem);
        m_framebuffer.attachments[1].mem = memory2;
    }
    void UShadowPass::setupRenderPass()
    {
        //所有的renderpass用到了两个附件, 就是上面创建的一个深度, 一个color附件
        Proxy.RenderPass.SetAttachmentNum(2);
        auto& AttachDesc = Proxy.RenderPass.GetAttachmentDesc();

        //阴影(还是着色?得看shader的实现)的颜色R16
        AttachDesc.SetFormat(0, Proxy.FrameBuffer.GetAttachment(0).Format);
        //这个附件会清理chip memory, 且将chip数据写入到system memory
        AttachDesc.SetLoadAndStore(0, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
        AttachDesc.SetLayout(0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //影子(还是着色?)深度
        AttachDesc.SetFormat(1, Proxy.FrameBuffer.GetAttachment(1).Format);
        //深度的话,也会清理chip memory, 但不会回写到system memory -- 根据UE, depth buffer确实也不会写回system memory
        AttachDesc.SetLoadAndStore(1, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
        AttachDesc.SetLayout(1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        //就一个pass
        Proxy.RenderPass.SetSubPassNum(1);
        FVulkanSubPass& SubPass = Proxy.RenderPass.GetSubPass(0);

        //两个附件, 一个ColorAttachment(输出), 一个DepthAttachment(深度)
        SubPass.SetReferenceNum(0, 1, 1);
        SubPass.SetColorReference(0, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        SubPass.SetDepthReference(1);
        SubPass.SetupSubPass();

        Proxy.RenderPass.SetDependencyNum(1);
        auto& Dependency = Proxy.RenderPass.GetDependency();
        Dependency.SetSubpass(0, VK_SUBPASS_EXTERNAL, 0);
        //就一个subpass, 依赖于之前pass的输出. 然后目标是自己的pass的输出
        Dependency.SetStageMask(
            0,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, //从上个管道输出最终颜色值
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT); //管道中的最后一个阶段，其中所有命令生成的操作完成执行
        //告知这个subpass会写颜色附件
        Dependency.SetAccessMask(0,
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, //通过PS输出向一个颜色附件写入数据
                                 0); //不访问
        Dependency.SetFlag(0, 0); // NOT BY REGION

        Proxy.RenderPass.CreateRenderPass(m_rhi->m_device);



        //TODO 兼容旧引擎
		auto pRenderPass = new VulkanRenderPass();
        pRenderPass->setResource(Proxy.RenderPass.GetVKRenderPass());
        m_framebuffer.render_pass = pRenderPass;
    }
    void UShadowPass::setupFramebuffer()
    {
        //使用initialize方法中手动创建的两个image view. 即自己已存的两个view
        Proxy.FrameBuffer.CreateFrameBuffer(m_rhi->m_device, Proxy.RenderPass.GetVKRenderPass());

        //TODO 兼容旧引擎
    	m_framebuffer.framebuffer = new VulkanFramebuffer();
        ((VulkanFramebuffer*)m_framebuffer.framebuffer)->setResource(Proxy.FrameBuffer.GetFrameBuffer());
    }
    void UShadowPass::setupDescriptorSetLayout()
    {
        //设置描述符布局
        Proxy.DescriptorSets.resize(1);

        // 3个binding, 对应shader中的layout(set = 0, binding = 0/1/2)
        // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC 可被动态写入的buffer, 允许单个set中的单个缓冲区高频地更新
        
        // 3个set都用于顶点阶段, 在mesh_directional_light_shadow.vert中分别是mvp, meshinstance, joint_matrices(动画)
        auto& Set = Proxy.DescriptorSets[0];
        //都用于顶点shader
        Set.SetBindingNum(3);
        Set.SetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.SetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.SetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.CreateDescriptorLayout(m_rhi->m_device);


        //TODO 兼容旧引擎
        m_descriptor_infos.resize(1);
        m_descriptor_infos[0].layout = new VulkanDescriptorSetLayout();
        ((VulkanDescriptorSetLayout*)m_descriptor_infos[0].layout)->setResource(Proxy.DescriptorSets[0].GetLayout());
    }
    void UShadowPass::setupPipelines()
    {
        Proxy.Pipelines.resize(1);
        FVulkanPipeline& Pipeline = Proxy.Pipelines[0];

        //设置管线布局, 管线布局用到了两个描述集布局
        std::vector<VkDescriptorSetLayout> DescLayouts = {
            Proxy.GetVkDescriptorLayout(0), 
            ((VulkanDescriptorSetLayout*)m_per_mesh_layout)->getResource()
        };
        Pipeline.CreateLayout(m_rhi->m_device, DescLayouts);

        //设置Shader
        FVulkanShader Shader;
        Shader.CreateShader(
            m_rhi->m_device, MESH_DIRECTIONAL_LIGHT_SHADOW_VERT, MESH_DIRECTIONAL_LIGHT_SHADOW_FRAG);
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        Shader.CreateStages(shader_stages);

        //设置状态
        FVulkanPipelineState State;
        State.SetupDefaultState();

        //设置顶点绑定和顶点属性
        //TODO 兼容旧引擎
        RHIVertexInputBindingDescription VBD = MeshVertex::getBindingDescriptions()[0];
        VkVertexInputBindingDescription  VB;
        VB.binding = VBD.binding;
        VB.stride  = VBD.stride;
        VB.inputRate = (VkVertexInputRate)VBD.inputRate;

        RHIVertexInputAttributeDescription VAD = MeshVertex::getAttributeDescriptions()[0];
        VkVertexInputAttributeDescription  VA  = {VAD.location, VAD.binding, (VkFormat)VAD.format, VAD.offset};

        std::vector<VkVertexInputBindingDescription> VertexBindings = { VB };
        State.SetVertexBindingDescription(VertexBindings);
        std::vector<VkVertexInputAttributeDescription> VertexAttributes = {VA};
        State.SetVertexAttributeDescription(VertexAttributes);

        //设置pipeline的个性化数据
        VkViewport viewport = Proxy.FrameBuffer.GetFullViewport();
        VkRect2D   scissor  = Proxy.FrameBuffer.GetFullScissor();
        State.SetViewportState(viewport, scissor);
        State.SetRasterFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE); //逆时针正方向

        State.SetColorBlendNum(1);
        State.SetColorBlendAttachmentEnable(0, VK_FALSE);
        State.SetColorBlendFactor(0, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
        State.SetAlphaBlendFactor(0, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
        State.SetupColorBlend();

        State.SetDepthState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE);
        State.SetStencilState(VK_FALSE);
        
        State.SetupNullDynamicState();

        //创建管线
        Pipeline.CreatePipeline(m_rhi->m_device, Proxy.RenderPass.GetVKRenderPass(), 0, shader_stages, State);


        //TODO 兼容旧数据
        m_render_pipelines.resize(1);
        VulkanPipeline* VP = new VulkanPipeline();
        VP->setResource(Pipeline.GetPipeline());
        m_render_pipelines[0].pipeline = VP;
        
        VulkanPipelineLayout* VPL = new VulkanPipelineLayout();
        VPL->setResource(Pipeline.GetLayout());
        m_render_pipelines[0].layout = VPL;

        Shader.DestroyShader(m_rhi->m_device);
    }
    //创建描述符集, 并把write和描述符集绑定起来
    void UShadowPass::setupDescriptorSet()
    {
        auto& Set = Proxy.DescriptorSets[0];
        VulkanDescriptorPool* Pool  = (VulkanDescriptorPool*)m_rhi->m_descriptor_pool;
        VkDescriptorPool      VPool = Pool->getResource();
        Set.CreateDescriptorSet(m_rhi->m_device, VPool);

        // Write和binding的数量一样
        // 创建了3个DescBufferInfo, 对应 VS shader的3个uniform(buffer), 奇怪的是在shader中没有uniform的标签
        
        //TODO 创建描述符要用的buffer
        VkBuffer RawBuffer =
            ((VulkanBuffer*)m_global_render_resource->_storage_buffer._global_upload_ringbuffer)->getResource();
        FVulkanBuffer Buffer = FVulkanBuffer(RawBuffer);


        //第一个是平行光的MVP
        VkDescriptorBufferInfo mesh_light_info =
            Buffer.CreateDescriptorBufferInfo(sizeof(MeshDirectionalLightShadowPerframeStorageBufferObject));
        Set.SetWriteBuffer(0, &mesh_light_info);

        //第二个每个Actor的Transform值. 直接传入了64个Actor位置的数组
        VkDescriptorBufferInfo meshes_transfor_info =
            Buffer.CreateDescriptorBufferInfo(sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject));
        Set.SetWriteBuffer(1, &meshes_transfor_info);

        //第三个是顶点动画
        VkDescriptorBufferInfo mesh_vertex_blending_info = Buffer.CreateDescriptorBufferInfo(
            sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject));
        Set.SetWriteBuffer(2, &mesh_vertex_blending_info);

        Set.UpdateDescriptorSets(m_rhi->m_device);

        // TODO 兼容旧引擎
        m_descriptor_infos[0].descriptor_set = new VulkanDescriptorSet();
        ((VulkanDescriptorSet*)m_descriptor_infos[0].descriptor_set)
            ->setResource(Proxy.DescriptorSets[0].GetDescriptorSet());
    }
    void UShadowPass::drawModel()
    {
        struct MeshNode
        {
            const Matrix4x4* model_matrix {nullptr};
            const Matrix4x4* joint_matrices {nullptr};
            uint32_t         joint_count {0};
        };

        //合批后的数据.
        std::map<VulkanPBRMaterial*, std::map<VulkanMesh*, std::vector<MeshNode>>>
            directional_light_mesh_drawcall_batch;

        // reorganize mesh
        for (RenderMeshNode& node : *(m_visiable_nodes.p_directional_light_visible_mesh_nodes))
        {
            auto& mesh_instanced = directional_light_mesh_drawcall_batch[node.ref_material];
            auto& mesh_nodes     = mesh_instanced[node.ref_mesh];

            MeshNode temp;
            temp.model_matrix = node.model_matrix;
            if (node.enable_vertex_blending)
            {
                temp.joint_matrices = node.joint_matrices;
                temp.joint_count    = node.joint_count;
            }

            mesh_nodes.push_back(temp);
        }

        VkCommandBuffer RawCommandBuffer;
        RawCommandBuffer = ((VulkanCommandBuffer*)m_rhi->m_current_command_buffer)->getResource();
        FVulkanCommandBuffer CB = FVulkanCommandBuffer(RawCommandBuffer);
        // Directional Light Shadow begin pass
        {
            Proxy.FrameBuffer.SetClearColorValue(0, {1.0f});
            Proxy.FrameBuffer.SetClearDepthValue(1, {1.0f, 0});
            CB.BeginRenderPass(Proxy.FrameBuffer, Proxy.RenderPass, Proxy.FrameBuffer.GetFullExtent());

            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Directional Light Shadow", color);
        }

        // 光源阴影可用
        if (m_rhi->isPointLightShadowEnabled())
        {
            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Mesh", color);


            CB.BindPipeline(Proxy.GetVkPipeline());

            FVulkanStagingBuffer_Storage StorageBuffer;
            StorageBuffer.Data = m_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer;
            StorageBuffer.Initialize(1024 * 1024 * 128, 3, 256);


            // IMPORTANT 赋值平行光的MVP
            uint32_t perframe_dynamic_offset = 0;
            MeshDirectionalLightShadowPerframeStorageBufferObject& Obj =
                StorageBuffer.GetObjectAtEndAddress<MeshDirectionalLightShadowPerframeStorageBufferObject>(
                    m_rhi->m_current_frame_index,
                    sizeof(MeshPerframeStorageBufferObject),
                    perframe_dynamic_offset);
            Obj = m_mesh_directional_light_shadow_perframe_storage_buffer_object;

            for (auto& [material, mesh_instanced] : directional_light_mesh_drawcall_batch)
            {
                // TODO: render from near to far

                //从这里看, 并没有进行模型的合并. 而且把材质球一样的模型放在一起渲染, 减少状态切换
                //描述符布局应该是材质球的成员变量
                for (auto& [mesh, mesh_nodes] : mesh_instanced)
                {
                    uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                    if (total_instance_count > 0)
                    {
                        // bind per mesh
                        VkDescriptorSet DS =
                            ((VulkanDescriptorSet*)mesh->mesh_vertex_blending_descriptor_set)->getResource();
                        //看顶点shader, 里面的布局用了两个set, 这里绑定的是索引为1, 即第二个
                        CB.BindDescriptorSets(
                            Proxy.GetVkPipelineLayout(), DS, 1);

                        // 绑定mesh的顶点和索引缓冲区
                        VkBuffer VertBuff = ((VulkanBuffer*)mesh->mesh_vertex_position_buffer)->getResource();
                        VkBuffer IndexBuff = ((VulkanBuffer*)mesh->mesh_index_buffer)->getResource();

                        CB.BindVertexBuffer(VertBuff);
                        CB.BindIndexBuffer(IndexBuff);

                        //平行光阴影, 每帧绘制64次DC
                        uint32_t drawcall_max_instance_count =
                            (sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances) /
                             sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances[0]));
                        //按64取整. 变为了64的倍数
                        uint32_t drawcall_count =
                            roundUp(total_instance_count, drawcall_max_instance_count) / drawcall_max_instance_count;

                        //不是. 剩下呢? 不绘制了
                        for (uint32_t drawcall_index = 0; drawcall_index < drawcall_count; ++drawcall_index)
                        {
                            uint32_t current_instance_count =
                                ((total_instance_count - drawcall_max_instance_count * drawcall_index) <
                                 drawcall_max_instance_count) ?
                                    (total_instance_count - drawcall_max_instance_count * drawcall_index) :
                                    drawcall_max_instance_count;

                            // perdrawcall storage buffer
                            // 又重复利用了. Stage内存的开始部分. 64个model的mvp
                            // 开始通过不断的偏移End值, 往buffer写值
                            uint32_t perdrawcall_dynamic_offset = 0;
                            MeshDirectionalLightShadowPerdrawcallStorageBufferObject&
                                perdrawcall_storage_buffer_object = StorageBuffer.GetObjectAtEndAddress<
                                    MeshDirectionalLightShadowPerdrawcallStorageBufferObject>(
                                    m_rhi->m_current_frame_index,
                                    sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject),
                                    perdrawcall_dynamic_offset);

                            //设置每个instance的model view matrix
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                perdrawcall_storage_buffer_object.mesh_instances[i].model_matrix =
                                    *mesh_nodes[drawcall_max_instance_count * drawcall_index + i].model_matrix;
                                perdrawcall_storage_buffer_object.mesh_instances[i].enable_vertex_blending =
                                    mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices ? 1.0 :
                                                                                                                  -1.0;
                            }

                            // per drawcall vertex blending storage buffer
                            uint32_t per_drawcall_vertex_blending_dynamic_offset;
                            bool     least_one_enable_vertex_blending = true;
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                if (!mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices)
                                {
                                    least_one_enable_vertex_blending = false;
                                    break;
                                }
                            }
                            if (least_one_enable_vertex_blending)
                            {
                                // 接着重复利用. Stage内存的开始部分
                                // 开始通过不断的偏移End值, 往buffer写值
                                MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject&
                                    per_drawcall_vertex_blending_storage_buffer_object =
                                        StorageBuffer.GetObjectAtEndAddress<
                                            MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject>(
                                            m_rhi->m_current_frame_index,
                                            sizeof(
                                                MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject),
                                            per_drawcall_vertex_blending_dynamic_offset);

                                for (uint32_t i = 0; i < current_instance_count; ++i)
                                {
                                    if (mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices)
                                    {
                                        for (uint32_t j = 0;
                                             j <
                                             mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_count;
                                             ++j)
                                        {
                                            per_drawcall_vertex_blending_storage_buffer_object
                                                .joint_matrices[s_mesh_vertex_blending_max_joint_count * i + j] =
                                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i]
                                                    .joint_matrices[j];
                                        }
                                    }
                                }
                            }
                            else
                            {
                                per_drawcall_vertex_blending_dynamic_offset = 0;
                            }

                            // bind perdrawcall
                            std::vector<uint32_t> dynamic_offsets = {perframe_dynamic_offset,
                                                           perdrawcall_dynamic_offset,
                                                           per_drawcall_vertex_blending_dynamic_offset};
                            CB.BindDescriptorSets(
                                Proxy.GetVkPipelineLayout(), Proxy.GetVkDescriptorSet(), dynamic_offsets);
                            CB.DrawIndexed(mesh->mesh_index_count, current_instance_count);
                        }
                    }
                }
            }

            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());
        }

        // Directional Light Shadow end pass
        {
            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());

            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }
} // namespace Piccolo
