#pragma once

#include "Renderer/RenderPass.h"

namespace Piccolo
{
    class RenderResourceBase;

    class UShadowPass : public URenderPass
    {
    public:
        void initialize(const FRenderPassInitInfo* init_info) override final;
        void postInitialize() override final;
        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;
        void draw() override final;

        //TODO 旧引擎
        void setPerMeshLayout(RHIDescriptorSetLayout* layout) { m_per_mesh_layout = layout; }

    private:
        void setupAttachments();
        void setupRenderPass();
        void setupFramebuffer();
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
        void drawModel();

    private:
        //TODO 旧引擎
        RHIDescriptorSetLayout* m_per_mesh_layout;
        MeshDirectionalLightShadowPerframeStorageBufferObject
            m_mesh_directional_light_shadow_perframe_storage_buffer_object;

        
    };
} // namespace Piccolo
