#pragma once

#include "runtime/Renderer/RenderPass.h"

namespace Piccolo
{
    class WindowUI;

    struct UIPassInitInfo : FRenderPassInitInfo
    {
        RHIRenderPass* render_pass;
    };

    class UIPass : public URenderPass
    {
    public:
        void initialize(const FRenderPassInitInfo* init_info) override final;
        void initializeUIRenderBackend(WindowUI* window_ui) override final;
        void draw() override final;

    private:
        void uploadFonts();

    private:
        WindowUI* m_window_ui;
    };
} // namespace Piccolo
