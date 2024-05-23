#pragma once

#include "RenderPipelineBase.h"

namespace Piccolo
{
    //引擎逻辑段的渲染器. 可以类比为UE的render线程的MobileRenderer
    class USceneRenderer : public URenderPipelineBase
    {
    public:
        virtual void initialize(FRenderPipelineInitInfo init_info) override final;

        virtual void deferredRender(std::shared_ptr<RHI>                rhi,
                                    std::shared_ptr<RenderResourceBase> render_resource) override final;

        void passUpdateAfterRecreateSwapchain();

        void setAxisVisibleState(bool state);

        void setSelectedAxis(size_t selected_axis);
    };
} // namespace Piccolo
