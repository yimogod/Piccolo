#pragma once
#include "runtime/Renderer/RenderPass.h"

namespace Piccolo
{
    struct ColorGradingPassInitInfo : FRenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView* input_attachment;
    };

    class ColorGradingPass : public URenderPass
    {
    public:
        void initialize(const FRenderPassInitInfo* init_info) override final;
        void draw() override final;

        void updateAfterFramebufferRecreate(RHIImageView* input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Piccolo
