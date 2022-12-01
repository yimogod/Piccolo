#pragma once

#include "runtime/function/render/interface/rhi.h"

namespace Piccolo
{
    class RHI;
    class RenderResourceBase;
    class WindowUI;

    struct FRenderPassInitInfo
    {};

    struct FRenderPassCommonInfo
    {
        std::shared_ptr<RHI>                rhi;
        std::shared_ptr<RenderResourceBase> render_resource;
    };

    class URenderPassBase
    {
    public:
        virtual void initialize(const FRenderPassInitInfo* init_info) = 0;
        virtual void postInitialize();
        virtual void setCommonInfo(FRenderPassCommonInfo common_info);
        virtual void preparePassData(std::shared_ptr<RenderResourceBase> render_resource);
        virtual void initializeUIRenderBackend(WindowUI* window_ui);

    protected:
        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderResourceBase> m_render_resource;
    };
} // namespace Piccolo
