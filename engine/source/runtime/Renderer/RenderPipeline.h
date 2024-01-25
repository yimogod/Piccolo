#pragma once

#include "RenderPipelineBase.h"

namespace Piccolo
{
    //引擎逻辑段的管线, 概念更广义. 表示整个引擎的渲染的全过程, 和Vulkan的Pipeline完全不一样
    class URenderPipeline : public URenderPipelineBase
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
