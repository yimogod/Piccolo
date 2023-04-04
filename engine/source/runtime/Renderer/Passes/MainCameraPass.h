#pragma once

#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"

#include "Renderer/RenderPass.h"

namespace Piccolo
{
    class RenderResourceBase;

    class UMainCameraPass : public URenderPass
    {
    public:
        // 1: per mesh layout
        // 2: global layout
        // 3: mesh per material layout
        // 4: sky box layout
        // 7: gbuffer lighting
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _mesh_global,
            _mesh_per_material,
            _skybox,
            _deferred_lighting,
            _layout_type_count
        };

        // 1. model
        // 2. sky box
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_mesh_gbuffer = 0,
            _render_pipeline_type_deferred_lighting,
            _render_pipeline_type_mesh_lighting,
            _render_pipeline_type_skybox,
            _render_pipeline_type_count
        };

        void initialize(const FRenderPassInitInfo* init_info) override final;

        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;

        void draw(ColorGradingPass& color_grading_pass,
                  ToneMappingPass&  tone_mapping_pass,
                  UIPass&           ui_pass,
                  CombineUIPass&    combine_ui_pass,
                  uint32_t          current_swapchain_image_index);

        RHIImageView* m_point_light_shadow_color_image_view;
        RHIImageView* m_directional_light_shadow_color_image_view;

        MeshPerframeStorageBufferObject m_mesh_perframe_storage_buffer_object;

        void updateAfterFramebufferRecreate();

        RHICommandBuffer* getRenderCommandBuffer();
    private:
        void setupAttachments();
        void setupRenderPass();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
        void setupFramebufferDescriptorSet();
        void setupSwapchainFramebuffers();

        void setupModelGlobalDescriptorSet();
        void setupSkyboxDescriptorSet();
        void setupGbufferLightingDescriptorSet();

        void drawMeshGbuffer();
        void drawDeferredLighting();

    private:
        std::vector<RHIFramebuffer*>  m_swapchain_framebuffers;
    };
} // namespace Piccolo
