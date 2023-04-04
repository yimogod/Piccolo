#pragma once

#include "runtime/function/render/render_common.h"
#include "RenderPassBase.h"
#include "RenderCore/VulkanProxy.h"
#include "RenderCore/VulkanPassProxy.h"
#include "runtime/function/render/render_resource.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>




namespace Piccolo
{
    class VulkanRHI;

    enum
    {
        E_main_camera_pass_gbuffer_a                     = 0,
        E_main_camera_pass_gbuffer_b                     = 1,
        E_main_camera_pass_gbuffer_c                     = 2,
        E_main_camera_pass_backup_buffer_odd             = 3,
        E_main_camera_pass_backup_buffer_even            = 4,
        E_main_camera_pass_post_process_buffer_odd       = 5,
        E_main_camera_pass_post_process_buffer_even      = 6,
        E_main_camera_pass_depth                         = 7,
        E_main_camera_pass_swap_chain_image              = 8,
        E_main_camera_pass_custom_attachment_count       = 5,
        E_main_camera_pass_post_process_attachment_count = 2,
        E_main_camera_pass_attachment_count              = 9,
    };

    enum
    {
        E_main_camera_subpass_basepass = 0,
        E_main_camera_subpass_deferred_lighting,
        E_main_camera_subpass_forward_lighting,
        E_main_camera_subpass_tone_mapping,
        E_main_camera_subpass_color_grading,
        E_main_camera_subpass_ui,
        E_main_camera_subpass_combine_ui,
        E_main_camera_subpass_count
    };

    struct FVisiableNodes
    {
        std::vector<RenderMeshNode>* p_directional_light_visible_mesh_nodes {nullptr};
        std::vector<RenderMeshNode>* p_point_lights_visible_mesh_nodes {nullptr};
        std::vector<RenderMeshNode>* p_main_camera_visible_mesh_nodes {nullptr};
    };

    //逻辑层面的渲染pass, 多个逻辑Pass构成逻辑层面的Pipeline.
    class URenderPass : public URenderPassBase
    {
    public:
        //逻辑上frame buffer用到的附件数据
        struct FFrameBufferAttachment
        {
            //图片数据
            RHIImage*        image;
            //存放图片的显存
            RHIDeviceMemory* mem;
            //对应的如何使用图片的View
            RHIImageView*    view;
            //图片格式
            RHIFormat        format;
        };

        // frame和渲染通道是一一对应, 本对象包含了帧缓存的所有数据
        // 包含帧缓存, render pass, attachments
        struct FFramebuffer
        {
            int             width;
            int             height;
            RHIFramebuffer* framebuffer;
            RHIRenderPass*  render_pass;

            // 自定义的附件数据, maincamerapass用到了7个.
            // 另外加上交换链所需的深度和swapchianimageview, 共9个给renderpass使用
            // 另外的例子是比如平行光, 就用到了3个
            std::vector<FFrameBufferAttachment> attachments;
        };

        // 描述符集合数据块. 包含布局和描述符集
        struct FDescriptor
        {
            RHIDescriptorSetLayout* layout;
            RHIDescriptorSet*       descriptor_set;
        };

        struct FRenderPipelineBase
        {
            RHIPipelineLayout* layout;
            RHIPipeline*       pipeline;
        };

        //这个pass可能用到的全局资源
        GlobalRenderResource* m_global_render_resource {nullptr};

        //这个pass的用到的shader的布局信息
        std::vector<FDescriptor>         m_descriptor_infos;
        // 4个渲染管线? 忘记啥意思了. 一个subpass对应一个pipeline?
        std::vector<FRenderPipelineBase> m_render_pipelines;
        //此渲染pass输出的framebuffer
        FFramebuffer                     m_framebuffer;

        void initialize(const FRenderPassInitInfo* init_info) override;
        void postInitialize() override;

        virtual void draw();

        virtual RHIRenderPass*                       getRenderPass() const;
        virtual std::vector<RHIImageView*>           getFramebufferImageViews() const;
        virtual std::vector<RHIDescriptorSetLayout*> getDescriptorSetLayouts() const;

        //可见对象
        static FVisiableNodes m_visiable_nodes;

    protected:
        // renderpass的vulkan数据
        UVulkanPassProxy Proxy;
        std::shared_ptr<UVulkanProxy> Vulkan;
    private:
    };
} // namespace Piccolo
