#pragma once

#include "runtime/core/math/vector2.h"
#include "runtime/function/render/render_pass_base.h"

#include <memory>
#include <vector>

#include "RenderPassBase.h"

namespace Piccolo
{
    class RHI;
    class RenderResourceBase;
    class WindowUI;

    struct FRenderPipelineInitInfo
    {
        bool                                enable_fxaa {false};
        std::shared_ptr<RenderResourceBase> render_resource;
    };

    // 逻辑渲染管线. 由多个渲染Pass构成. 这个和vulkan的pipeline的概念不一样.
    // vulkan的pipeline是针对某个subpass的. 相当于微管线;)
    class URenderPipelineBase
    {
        friend class URenderSystem;

    public:
        virtual ~URenderPipelineBase() {}

        virtual void clear() {};

        virtual void initialize(FRenderPipelineInitInfo init_info) = 0;

        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);
        virtual void deferredRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

        void             initializeUIRenderBackend(WindowUI* window_ui);
        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) = 0;

        std::shared_ptr<RHI> m_rhi;
    public:
        
        //构成此渲染的各个pass
        std::shared_ptr<URenderPassBase> m_directional_light_pass;
        std::shared_ptr<RenderPassBase> m_point_light_shadow_pass;
        std::shared_ptr<RenderPassBase> m_main_camera_pass;
        std::shared_ptr<RenderPassBase> m_color_grading_pass;
        std::shared_ptr<RenderPassBase> m_fxaa_pass;
        std::shared_ptr<RenderPassBase> m_tone_mapping_pass;
        std::shared_ptr<RenderPassBase> m_ui_pass;
        std::shared_ptr<RenderPassBase> m_combine_ui_pass;
        std::shared_ptr<RenderPassBase> m_pick_pass;
        std::shared_ptr<RenderPassBase> m_particle_pass;

    };
} // namespace Piccolo
