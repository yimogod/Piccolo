#include "SceneRenderer.h"

#include "Passes/MainCameraPass.h"
#include "Passes/ShadowPass.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

#include "runtime/function/render/passes/color_grading_pass.h"
#include "runtime/function/render/passes/combine_ui_pass.h"
#include "runtime/function/render/passes/tone_mapping_pass.h"
#include "runtime/function/render/passes/ui_pass.h"

#include "runtime/function/render/debugdraw/debug_draw_manager.h"

#include "runtime/core/base/macro.h"

namespace Piccolo
{
    void USceneRenderer::initialize(FRenderPipelineInitInfo init_info)
    {
        m_shadow_pass             = std::make_shared<UShadowPass>();
        m_main_camera_pass        = std::make_shared<UMainCameraPass>();
        m_tone_mapping_pass       = std::make_shared<ToneMappingPass>();
        m_color_grading_pass      = std::make_shared<ColorGradingPass>();
        m_ui_pass                 = std::make_shared<UIPass>();
        m_combine_ui_pass         = std::make_shared<CombineUIPass>();

        FRenderPassCommonInfo pass_common_info;
        pass_common_info.rhi             = m_rhi;
        pass_common_info.render_resource = init_info.render_resource;
        pass_common_info.Vulkan = Vulkan;

        m_shadow_pass->setCommonInfo(pass_common_info);
        m_main_camera_pass->setCommonInfo(pass_common_info);
        m_tone_mapping_pass->setCommonInfo(pass_common_info);
        m_color_grading_pass->setCommonInfo(pass_common_info);
        m_ui_pass->setCommonInfo(pass_common_info);
        m_combine_ui_pass->setCommonInfo(pass_common_info);

        m_shadow_pass->initialize(nullptr);

        std::shared_ptr<UMainCameraPass> main_camera_pass = std::static_pointer_cast<UMainCameraPass>(m_main_camera_pass);
        std::shared_ptr<URenderPass>     _main_camera_pass = std::static_pointer_cast<URenderPass>(m_main_camera_pass);

        main_camera_pass->m_directional_light_shadow_color_image_view =
            std::static_pointer_cast<URenderPass>(m_shadow_pass)->Framebuffer.attachments[0].view;

        m_main_camera_pass->initialize(nullptr);

        std::vector<RHIDescriptorSetLayout*> descriptor_layouts = _main_camera_pass->getDescriptorSetLayouts();
        std::static_pointer_cast<UShadowPass>(m_shadow_pass)
            ->setPerMeshLayout(descriptor_layouts[UMainCameraPass::LayoutType::_per_mesh]);

        m_shadow_pass->postInitialize();

        ToneMappingPassInitInfo tone_mapping_init_info;
        tone_mapping_init_info.render_pass = _main_camera_pass->getRenderPass();
        tone_mapping_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[E_main_camera_pass_backup_buffer_odd];
        m_tone_mapping_pass->initialize(&tone_mapping_init_info);

        ColorGradingPassInitInfo color_grading_init_info;
        color_grading_init_info.render_pass = _main_camera_pass->getRenderPass();
        color_grading_init_info.input_attachment =
            _main_camera_pass->getFramebufferImageViews()[E_main_camera_pass_backup_buffer_even];
        m_color_grading_pass->initialize(&color_grading_init_info);

        UIPassInitInfo ui_init_info;
        ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        m_ui_pass->initialize(&ui_init_info);

        CombineUIPassInitInfo combine_ui_init_info;
        combine_ui_init_info.render_pass = _main_camera_pass->getRenderPass();
        combine_ui_init_info.scene_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[E_main_camera_pass_backup_buffer_odd];
        combine_ui_init_info.ui_input_attachment =
            _main_camera_pass->getFramebufferImageViews()[E_main_camera_pass_backup_buffer_even];
        m_combine_ui_pass->initialize(&combine_ui_init_info);
    }

    void USceneRenderer::deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        //每帧进行调用

        VulkanRHI*      vulkan_rhi      = static_cast<VulkanRHI*>(rhi.get());
        RenderResource* vulkan_resource = static_cast<RenderResource*>(render_resource.get());

        //绘制前, 重置当前三缓冲需要的staging buffer
        vulkan_resource->resetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        //等待cpu/gpu同步
        vulkan_rhi->waitForFences();

        vulkan_rhi->resetCommandPool();

        bool recreate_swapchain = vulkan_rhi->prepareBeforePass(std::bind(&USceneRenderer::passUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)return;

        //平行光阴影pass
        static_cast<UShadowPass*>(m_shadow_pass.get())->draw();

        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        UIPass&           ui_pass            = *(static_cast<UIPass*>(m_ui_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));

        static_cast<UMainCameraPass*>(m_main_camera_pass.get())
            ->draw(color_grading_pass,
                   tone_mapping_pass,
                   ui_pass,
                   combine_ui_pass,
                   vulkan_rhi->m_current_swapchain_image_index);
                   
        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->submitRendering(std::bind(&USceneRenderer::passUpdateAfterRecreateSwapchain, this));
    }

    void USceneRenderer::passUpdateAfterRecreateSwapchain()
    {
        UMainCameraPass&   main_camera_pass   = *(static_cast<UMainCameraPass*>(m_main_camera_pass.get()));
        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_color_grading_pass.get()));
        ToneMappingPass&  tone_mapping_pass  = *(static_cast<ToneMappingPass*>(m_tone_mapping_pass.get()));
        CombineUIPass&    combine_ui_pass    = *(static_cast<CombineUIPass*>(m_combine_ui_pass.get()));

        main_camera_pass.updateAfterFramebufferRecreate();
        tone_mapping_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[E_main_camera_pass_backup_buffer_odd]);
        color_grading_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[E_main_camera_pass_backup_buffer_even]);
        combine_ui_pass.updateAfterFramebufferRecreate(
            main_camera_pass.getFramebufferImageViews()[E_main_camera_pass_backup_buffer_odd],
            main_camera_pass.getFramebufferImageViews()[E_main_camera_pass_backup_buffer_even]);
        g_runtime_global_context.m_debugdraw_manager->updateAfterRecreateSwapchain();
    }

    void USceneRenderer::setAxisVisibleState(bool state)
    {
        UMainCameraPass& main_camera_pass = *(static_cast<UMainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_is_show_axis  = state;
    }

    void USceneRenderer::setSelectedAxis(size_t selected_axis)
    {
        UMainCameraPass& main_camera_pass = *(static_cast<UMainCameraPass*>(m_main_camera_pass.get()));
        main_camera_pass.m_selected_axis = selected_axis;
    }
} // namespace Piccolo
