#pragma once

#include "RenderPipelineBase.h"

namespace Piccolo
{
    class URenderPipeline : public URenderPipelineBase
    {
    public:
        virtual void initialize(FRenderPipelineInitInfo init_info) override final;

        virtual void deferredRender(std::shared_ptr<RHI>                rhi,
                                    std::shared_ptr<RenderResourceBase> render_resource) override final;

        void passUpdateAfterRecreateSwapchain();

        virtual uint32_t getGuidOfPickedMesh(const Vector2& picked_uv) override final;

        void setAxisVisibleState(bool state);

        void setSelectedAxis(size_t selected_axis);
    };
} // namespace Piccolo
