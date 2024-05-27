#include "ShadowPass.h"

#include "function/render/render_helper.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_mesh.h"

#include <mesh_directional_light_shadow_frag.h>
#include <mesh_directional_light_shadow_vert.h>

namespace Piccolo
{
    void UShadowPass::initialize(const FRenderPassInitInfo* init_info)
    {
        URenderPass::initialize(nullptr);

    	Packet.FrameBuffer.Width  = s_directional_light_shadow_map_dimension;
        Packet.FrameBuffer.Height = s_directional_light_shadow_map_dimension;

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
        FVulkanImageView View_0;
        View_0.SetSize(Packet.FrameBuffer.Width, Packet.FrameBuffer.Height);
        View_0.SetFormat(VK_FORMAT_R32_SFLOAT);
        View_0.SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);//用于采样和Ps输出--即shader的输入输出
        View_0.CreateImageView_Color(Vulkan->Device2);
        Packet.FrameBuffer.AddAttachment(View_0);

        // depth
        FVulkanImageView View_1;
        View_1.SetSize(Packet.FrameBuffer.Width, Packet.FrameBuffer.Height);
        View_1.SetFormat(Vulkan->DepthFormat);
        View_1.SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
        View_1.CreateImageView_Depth(Vulkan->Device2);
        Packet.FrameBuffer.AddAttachment(View_1);




        ////////////////////////////// TODO 兼容代码  ///////////////////////
        // color and depth
        Framebuffer.attachments.resize(2);

        // color
        Framebuffer.attachments[0].format = (RHIFormat)View_0.GetFormat();
        
        auto image = new VulkanImage();
        image->setResource(View_0.GetVkImage());
        Framebuffer.attachments[0].image = image;

        auto image_view = new VulkanImageView();
        image_view->setResource(View_0.GetVkView());
        Framebuffer.attachments[0].view  = image_view;

        auto memory = new VulkanDeviceMemory();
        memory->setResource(View_0.GetVkMem());
        Framebuffer.attachments[0].mem = memory;


        // depth
        Framebuffer.attachments[1].format = (RHIFormat)View_1.GetFormat();

        auto image2 = new VulkanImage();
        image2->setResource(View_1.GetVkImage());
        Framebuffer.attachments[1].image = image2;

        auto image_view2 = new VulkanImageView();
        image_view2->setResource(View_1.GetVkView());
        Framebuffer.attachments[1].view = image_view2;

        auto memory2 = new VulkanDeviceMemory();
        memory->setResource(View_1.GetVkMem());
        Framebuffer.attachments[1].mem = memory2;
    }

    void UShadowPass::setupRenderPass()
    {
        //所有的renderpass用到了两个附件, 就是上面创建的一个深度, 一个color附件
        Packet.RenderPass.SetAttachmentNum(2);
        auto& AttachDesc = Packet.RenderPass.GetAttachmentDesc();

        //阴影(还是着色?得看shader的实现)的颜色R16
        AttachDesc.SetFormat(0, Packet.FrameBuffer.GetAttachmentFormat(0));
        //这个附件会清理chip memory, 且将chip数据写入到system memory
        AttachDesc.SetLoadAndStore(0, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
        AttachDesc.SetLayout(0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //影子(还是着色?)深度
        AttachDesc.SetFormat(1, Packet.FrameBuffer.GetAttachmentFormat(1));
        //深度的话,也会清理chip memory, 但不会回写到system memory -- 根据UE, depth buffer确实也不会写回system memory
        AttachDesc.SetLoadAndStore(1, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
        AttachDesc.SetLayout(1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        //就一个pass
        Packet.RenderPass.SetSubPassNum(1);
        FVulkanSubPass& SubPass = Packet.RenderPass.GetSubPass(0);

        //两个附件, 一个ColorAttachment(输出), 一个DepthAttachment(深度)
        SubPass.SetReferenceNum(0, 1, 1);
        SubPass.SetColorReference(0, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        SubPass.SetDepthReference(1);
        SubPass.SetupSubPass();

        Packet.RenderPass.SetDependencyNum(1);
        auto& Dependency = Packet.RenderPass.GetDependency();
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

        Packet.RenderPass.CreateRenderPass(Vulkan->Device2);



        //TODO 兼容旧引擎
		auto pRenderPass = new VulkanRenderPass();
        pRenderPass->setResource(Packet.RenderPass.GetVkRenderPass());
        Framebuffer.render_pass = pRenderPass;
    }

    void UShadowPass::setupFramebuffer()
    {
        //使用initialize方法中手动创建的两个image view. 即自己已存的两个view
        Packet.FrameBuffer.CreateFrameBuffer(Vulkan->Device2, Packet.RenderPass);

        //TODO 兼容旧引擎
    	Framebuffer.framebuffer = new VulkanFramebuffer();
        ((VulkanFramebuffer*)Framebuffer.framebuffer)->setResource(Packet.FrameBuffer.GetVkFrameBuffer());
    }

    void UShadowPass::setupDescriptorSetLayout()
    {
        //设置描述符布局
        Packet.SetDescriptorSetNum(1);

        // 3个binding, 对应shader中的layout(set = 0, binding = 0/1/2)
        // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC 可被动态写入的buffer, 允许单个set中的单个缓冲区高频地更新

        // 3个set都用于顶点阶段, 在mesh_directional_light_shadow.vert中分别是mvp, meshinstance, joint_matrices(动画)
        auto& Set = Packet.GetDescriptorSet(0);
        //都用于顶点shader
        Set.SetBindingNum(3);
        Set.SetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.SetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.SetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
        Set.CreateDescriptorLayout(Vulkan->Device2);


        //TODO 兼容旧引擎
        m_descriptor_infos.resize(1);
        m_descriptor_infos[0].layout = new VulkanDescriptorSetLayout();
        ((VulkanDescriptorSetLayout*)m_descriptor_infos[0].layout)->setResource(Packet.DescriptorSets[0].GetVkLayout());
    }

    void UShadowPass::setupPipelines()
    {
        Packet.SetPipelineNum(1);
        FVulkanPipeline& Pipeline = Packet.GetPipeline(0);

        //设置管线布局, 管线布局用到了两个描述集布局
        std::vector<VkDescriptorSetLayout> DescLayouts = {
            Packet.GetVkDescriptorLayout(0), 
            ((VulkanDescriptorSetLayout*)m_per_mesh_layout)->getResource()
        };
        Pipeline.CreateLayout(Vulkan->Device2, DescLayouts);
        //设置Shader
        Pipeline.CreateShader(Vulkan->Device2, MESH_DIRECTIONAL_LIGHT_SHADOW_VERT, MESH_DIRECTIONAL_LIGHT_SHADOW_FRAG);

        //设置状态
        FVulkanPipelineState State;
        State.SetupDefaultState();

        //设置顶点绑定和顶点属性
        RHIVertexInputBindingDescription VBD = MeshVertex::getBindingDescriptions()[0];
        VkVertexInputBindingDescription  VB;
        VB.binding = VBD.binding;
        VB.stride  = VBD.stride;
        VB.inputRate = (VkVertexInputRate)VBD.inputRate;
        //设置顶点绑定给管线状态
        std::vector<VkVertexInputBindingDescription> VertexBindings = { VB };
        State.SetVertexBindingDescription(VertexBindings);

        RHIVertexInputAttributeDescription VAD = MeshVertex::getAttributeDescriptions()[0];
        VkVertexInputAttributeDescription  VA  = {VAD.location, VAD.binding, (VkFormat)VAD.format, VAD.offset};
        //设置顶点数据给管线状态
        std::vector<VkVertexInputAttributeDescription> VertexAttributes = {VA};
        State.SetVertexAttributeDescription(VertexAttributes);

        //设置pipeline的个性化数据
        VkViewport viewport = Packet.FrameBuffer.GetFullViewport();
        VkRect2D scissor = Packet.FrameBuffer.GetFullScissor();
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
        Pipeline.CreatePipeline(Vulkan->Device2, Packet.RenderPass, 0, State);


        //TODO 兼容旧数据
        Pipelines.resize(1);
        VulkanPipeline* VP = new VulkanPipeline();
        VP->setResource(Pipeline.GetVkPipeline());
        Pipelines[0].pipeline = VP;
        
        VulkanPipelineLayout* VPL = new VulkanPipelineLayout();
        VPL->setResource(Pipeline.GetVkLayout());
        Pipelines[0].layout = VPL;



        Pipeline.DestroyShader(Vulkan->Device2);
    }

    //创建描述符集, 并把write和描述符集绑定起来
    void UShadowPass::setupDescriptorSet()
    {
        auto& Set = Packet.GetDescriptorSet(0);
        Set.CreateDescriptorSet(Vulkan->Device2, Vulkan->DescriptorPool2);

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

        //Set的数据已经填充完毕. 可以通过write同步了
        Set.UpdateDescriptorSets(Vulkan->Device2);

        // TODO 兼容旧引擎
        m_descriptor_infos[0].descriptor_set = new VulkanDescriptorSet();
        ((VulkanDescriptorSet*)m_descriptor_infos[0].descriptor_set)
            ->setResource(Packet.DescriptorSets[0].GetVkDescriptorSet());
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
        //先根据渲染状态合批. 然后根据mesh合批. 左后是这个mesh的transform数据
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
            Packet.FrameBuffer.SetClearColorValue(0, {1.0f});
            Packet.FrameBuffer.SetClearDepthValue(1, {1.0f, 0});
            CB.BeginRenderPass(Packet.FrameBuffer, Packet.RenderPass, Packet.FrameBuffer.GetFullExtent());

            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Directional Light Shadow", color);
        }

        // Draw Mesh
        {
            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "Mesh", color);

            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[0].pipeline);

            // perframe storage buffer
            uint32_t perframe_dynamic_offset =
                roundUp(m_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
            m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                perframe_dynamic_offset + sizeof(MeshPerframeStorageBufferObject);
            assert(m_global_render_resource->_storage_buffer
                       ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                   (m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                    m_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

            MeshDirectionalLightShadowPerframeStorageBufferObject& perframe_storage_buffer_object =
                (*reinterpret_cast<MeshDirectionalLightShadowPerframeStorageBufferObject*>(
                    reinterpret_cast<uintptr_t>(
                        m_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
                    perframe_dynamic_offset));
            perframe_storage_buffer_object = m_mesh_directional_light_shadow_perframe_storage_buffer_object;

            for (auto& [material, mesh_instanced] : directional_light_mesh_drawcall_batch)
            {
                // TODO: render from near to far

                for (auto& [mesh, mesh_nodes] : mesh_instanced)
                {
                    uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                    if (total_instance_count > 0)
                    {
                        // bind per mesh
                        m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                                                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                                                        Pipelines[0].layout,
                                                        1,
                                                        1,
                                                        &mesh->mesh_vertex_blending_descriptor_set,
                                                        0,
                                                        NULL);

                        RHIBuffer*     vertex_buffers[] = {mesh->mesh_vertex_position_buffer};
                        RHIDeviceSize offsets[]        = {0};
                        m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, vertex_buffers, offsets);
                        m_rhi->cmdBindIndexBufferPFN(m_rhi->getCurrentCommandBuffer(), mesh->mesh_index_buffer, 0, RHI_INDEX_TYPE_UINT16);

                        uint32_t drawcall_max_instance_count =
                            (sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances) /
                             sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject::mesh_instances[0]));
                        uint32_t drawcall_count =
                            roundUp(total_instance_count, drawcall_max_instance_count) / drawcall_max_instance_count;

                        for (uint32_t drawcall_index = 0; drawcall_index < drawcall_count; ++drawcall_index)
                        {
                            uint32_t current_instance_count =
                                ((total_instance_count - drawcall_max_instance_count * drawcall_index) <
                                 drawcall_max_instance_count) ?
                                    (total_instance_count - drawcall_max_instance_count * drawcall_index) :
                                    drawcall_max_instance_count;

                            // perdrawcall storage buffer
                            uint32_t perdrawcall_dynamic_offset =
                                roundUp(m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                                        m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                                perdrawcall_dynamic_offset +
                                sizeof(MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
                            assert(m_global_render_resource->_storage_buffer
                                       ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                                   (m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                                    m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

                            MeshDirectionalLightShadowPerdrawcallStorageBufferObject&
                                perdrawcall_storage_buffer_object =
                                    (*reinterpret_cast<MeshDirectionalLightShadowPerdrawcallStorageBufferObject*>(
                                        reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                                                        ._global_upload_ringbuffer_memory_pointer) +
                                        perdrawcall_dynamic_offset));
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
                                per_drawcall_vertex_blending_dynamic_offset = roundUp(
                                    m_global_render_resource->_storage_buffer
                                        ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()],
                                    m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                                m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] =
                                    per_drawcall_vertex_blending_dynamic_offset +
                                    sizeof(MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject);
                                assert(m_global_render_resource->_storage_buffer
                                           ._global_upload_ringbuffers_end[m_rhi->getCurrentFrameIndex()] <=
                                       (m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_begin[m_rhi->getCurrentFrameIndex()] +
                                        m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffers_size[m_rhi->getCurrentFrameIndex()]));

                                MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject&
                                    per_drawcall_vertex_blending_storage_buffer_object =
                                        (*reinterpret_cast<
                                            MeshDirectionalLightShadowPerdrawcallVertexBlendingStorageBufferObject*>(
                                            reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                                                            ._global_upload_ringbuffer_memory_pointer) +
                                            per_drawcall_vertex_blending_dynamic_offset));
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
                            uint32_t dynamic_offsets[3] = {perframe_dynamic_offset,
                                                           perdrawcall_dynamic_offset,
                                                           per_drawcall_vertex_blending_dynamic_offset};
                            m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                                                            RHI_PIPELINE_BIND_POINT_GRAPHICS,
                                                            Pipelines[0].layout,
                                                            0,
                                                            1,
                                                            &m_descriptor_infos[0].descriptor_set,
                                                            (sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0])),
                                                            dynamic_offsets);
                            m_rhi->cmdDrawIndexedPFN(m_rhi->getCurrentCommandBuffer(),
                                                     mesh->mesh_index_count,
                                                     current_instance_count,
                                                     0,
                                                     0,
                                                     0);
                        }
                    }
                }
            }

            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());
        }





        // Directional Light Shadow end pass
        {
            m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());

            CB.EndRenderPass();
            //m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }
} // namespace Piccolo
