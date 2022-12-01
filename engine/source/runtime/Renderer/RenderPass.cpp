#include "RenderPass.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"
#include "runtime/function/render/render_resource.h"

Piccolo::FVisiableNodes Piccolo::URenderPass::m_visiable_nodes;

namespace Piccolo
{
    void URenderPass::initialize(const FRenderPassInitInfo* init_info)
    {
        m_global_render_resource =
            &(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
    }
    void URenderPass::draw() {}

    void URenderPass::postInitialize() {}

    RHIRenderPass* URenderPass::getRenderPass() const { return m_framebuffer.render_pass; }

    std::vector<RHIImageView*> URenderPass::getFramebufferImageViews() const
    {
        std::vector<RHIImageView*> image_views;
        for (auto& attach : m_framebuffer.attachments)
        {
            image_views.push_back(attach.view);
        }
        return image_views;
    }

    std::vector<RHIDescriptorSetLayout*> URenderPass::getDescriptorSetLayouts() const
    {
        std::vector<RHIDescriptorSetLayout*> layouts;
        for (auto& desc : m_descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
} // namespace Piccolo
