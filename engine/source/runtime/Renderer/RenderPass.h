#pragma once

#include "runtime/function/render/render_common.h"
#include "RenderPassBase.h"
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
        E_main_camera_subpass_fxaa,
        E_main_camera_subpass_ui,
        E_main_camera_subpass_combine_ui,
        E_main_camera_subpass_count
    };

    struct FVisiableNodes
    {
        std::vector<RenderMeshNode>* p_directional_light_visible_mesh_nodes {nullptr};
        std::vector<RenderMeshNode>* p_point_lights_visible_mesh_nodes {nullptr};
        std::vector<RenderMeshNode>* p_main_camera_visible_mesh_nodes {nullptr};
        RenderAxisNode*              p_axis_node {nullptr};
    };

    class URenderPass : public URenderPassBase
    {
    public:
        struct FFrameBufferAttachment
        {
            RHIImage*        image;
            RHIDeviceMemory* mem;
            RHIImageView*    view;
            RHIFormat        format;
        };

        struct FFramebuffer
        {
            int             width;
            int             height;
            RHIFramebuffer* framebuffer;
            RHIRenderPass*  render_pass;

            std::vector<FFrameBufferAttachment> attachments;
        };

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

        GlobalRenderResource* m_global_render_resource {nullptr};

        std::vector<FDescriptor>         m_descriptor_infos;
        std::vector<FRenderPipelineBase> m_render_pipelines;
        FFramebuffer                     m_framebuffer;

        void initialize(const FRenderPassInitInfo* init_info) override;
        void postInitialize() override;

        virtual void draw();

        virtual RHIRenderPass*                       getRenderPass() const;
        virtual std::vector<RHIImageView*>           getFramebufferImageViews() const;
        virtual std::vector<RHIDescriptorSetLayout*> getDescriptorSetLayouts() const;

        static FVisiableNodes m_visiable_nodes;

    private:
    };
} // namespace Piccolo
