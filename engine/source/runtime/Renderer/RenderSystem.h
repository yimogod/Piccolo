#pragma once

#include <array>
#include <memory>
#include <optional>
#include "RenderCore/VulkanRHI.h"

#include "runtime/function/render/render_entity.h"
#include "runtime/function/render/render_guid_allocator.h"
#include "runtime/function/render/render_swap_context.h"
#include "runtime/function/render/render_type.h"
#include "RenderPipelineBase.h"
#include "RenderCore/VulkanProxy.h"

namespace Piccolo
{
    class WindowSystem;
    class RHI;
    class RenderResourceBase;
    class URenderPipelineBase;
    class RenderScene;
    class RenderCamera;
    class WindowUI;
    class DebugDrawManager;

    struct FRenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<DebugDrawManager> debugdraw_manager;
    };

    struct FEngineContentViewport
    {
        float x { 0.f};
        float y { 0.f};
        float width { 0.f};
        float height { 0.f};
    };

    class URenderSystem
    {
    public:
        URenderSystem() = default;
        ~URenderSystem();

        void initialize(FRenderSystemInitInfo init_info);
        void tick(float delta_time);
        void clear();

        void                          swapLogicRenderData();
        RenderSwapContext&            getSwapContext();
        std::shared_ptr<RenderCamera> getRenderCamera() const;
        std::shared_ptr<RHI>          getRHI() const;

        void      initializeUIRenderBackend(WindowUI* window_ui);
        void      updateEngineContentViewport(float offset_x, float offset_y, float width, float height);
        GObjectID getGObjectIDByMeshID(uint32_t mesh_id) const;

        FEngineContentViewport getEngineContentViewport() const;

        void createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas);
        void setVisibleAxis(std::optional<RenderEntity> axis);
        void setSelectedAxis(size_t selected_axis);
        GuidAllocator<GameObjectPartId>& getGOInstanceIdAllocator();
        GuidAllocator<MeshSourceDesc>&   getMeshAssetIdAllocator();

        void clearForLevelReloading();

    private:
        RenderSwapContext m_swap_context;

        std::shared_ptr<UVulkanProxy>       Vulkan;
        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderCamera>       m_render_camera;
        std::shared_ptr<RenderScene>        m_render_scene;
        std::shared_ptr<RenderResourceBase> m_render_resource;
        std::shared_ptr<URenderPipelineBase> Renderer;

        //从逻辑线程获取数据到渲染线程
        void processSwapData();
    };
} // namespace Piccolo