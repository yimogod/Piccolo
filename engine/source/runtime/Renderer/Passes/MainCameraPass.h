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
        // 5: axis layout
        // 6: billboard type particle layout
        // 7: gbuffer lighting
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _mesh_global,
            _mesh_per_material,
            _skybox,
            _axis,
            _deferred_lighting,
            _layout_type_count
        };

        // 1. model
        // 2. sky box
        // 3. axis
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_mesh_gbuffer = 0,
            _render_pipeline_type_deferred_lighting,
            _render_pipeline_type_mesh_lighting,
            _render_pipeline_type_skybox,
            _render_pipeline_type_axis,
            _render_pipeline_type_count
        };

        void initialize(const FRenderPassInitInfo* init_info) override final;

        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;

        void draw(ColorGradingPass& color_grading_pass,
                  ToneMappingPass&  tone_mapping_pass,
                  UIPass&           ui_pass,
                  CombineUIPass&    combine_ui_pass,
                  uint32_t          current_swapchain_image_index);

        RHIImageView* m_directional_light_shadow_color_image_view;

        bool                            m_is_show_axis {false};
        size_t                          m_selected_axis {3};
        MeshPerframeStorageBufferObject m_mesh_perframe_storage_buffer_object;
        AxisStorageBufferObject         m_axis_storage_buffer_object;

        void updateAfterFramebufferRecreate();

        RHICommandBuffer* getRenderCommandBuffer();
    private:
        void SetupAttachments();
        void SetupRenderPass();
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
        void SetupFramebufferDescriptorSet();
        void SetupSwapchainFramebuffers();

        void SetupModelGlobalDescriptorSet();
        void SetupSkyboxDescriptorSet();
        void SetupAxisDescriptorSet();
        void SetupGbufferLightingDescriptorSet();

        void DrawMeshGbuffer();
        void DrawDeferredLighting();
        void drawAxis();

    private:
        std::vector<RHIFramebuffer*>  m_swapchain_framebuffers;
    };
} // namespace Piccolo
