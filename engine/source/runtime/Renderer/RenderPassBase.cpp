#include "RenderPassBase.h"

#include "runtime/core/base/macro.h"

namespace Piccolo
{
    void URenderPassBase::postInitialize() {}
    void URenderPassBase::preparePassData(std::shared_ptr<RenderResourceBase> render_resource) {}
    void URenderPassBase::initializeUIRenderBackend(WindowUI* window_ui) {}
} // namespace Piccolo
