#include "RenderPassBase.h"

#include "runtime/core/base/macro.h"

namespace Piccolo
{
    void URenderPassBase::postInitialize() {}
    void URenderPassBase::setCommonInfo(FRenderPassCommonInfo common_info)
    {
        m_rhi             = std::dynamic_pointer_cast<VulkanRHI>(common_info.rhi);
        m_render_resource = common_info.render_resource;
    }
    void URenderPassBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource) {}
    void URenderPassBase::initializeUIRenderBackend(WindowUI* window_ui) {}
} // namespace Piccolo
