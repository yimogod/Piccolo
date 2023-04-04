#pragma once

#include "runtime/Renderer/RenderPass.h"

namespace Piccolo
{
    struct CombineUIPassInitInfo : FRenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView*  scene_input_attachment;
        RHIImageView*  ui_input_attachment;
    };

    class CombineUIPass : public URenderPass
    {
    public:
        void initialize(const FRenderPassInitInfo* init_info) override final;
        void draw() override final;

        void updateAfterFramebufferRecreate(RHIImageView* scene_input_attachment, RHIImageView* ui_input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Piccolo
