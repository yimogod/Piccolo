#include "RenderSystem.h"
#include "RenderPass.h"

#include "Passes/MainCameraPass.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_pass.h"
#include "RenderPipeline.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/window_system.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/debugdraw/debug_draw_manager.h"

#include "runtime/function/render/passes/particle_pass.h"

#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

namespace Piccolo
{


URenderSystem::~URenderSystem()
{
    clear();
}

void URenderSystem::initialize(FRenderSystemInitInfo init_info)
{
    std::shared_ptr<ConfigManager> config_manager = g_runtime_global_context.m_config_manager;
    ASSERT(config_manager);
    std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
    ASSERT(asset_manager);

    // render context initialize
    RHIInitInfo rhi_init_info;
    rhi_init_info.window_system = init_info.window_system;

    //初始化 rhi
    m_rhi = std::make_shared<VulkanRHI>();
    m_rhi->initialize(rhi_init_info);

    Vulkan         = std::make_shared<UVulkanProxy>();
    Vulkan->Device = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_device;
    Vulkan->Gpu    = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_physical_device;

    // global rendering resource
    GlobalRenderingRes global_rendering_res;
    const std::string& global_rendering_res_url = config_manager->getGlobalRenderingResUrl();
    asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

    // upload ibl, color grading textures
    LevelResourceDesc level_resource_desc;
    level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
    level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map   = global_rendering_res.m_skybox_specular_map;
    level_resource_desc.m_ibl_resource_desc.m_brdf_map              = global_rendering_res.m_brdf_map;
    level_resource_desc.m_color_grading_resource_desc.m_color_grading_map =
        global_rendering_res.m_color_grading_map;

    //加载引擎默认用到的资源, 比如天空盒
    m_render_resource = std::make_shared<RenderResource>();
    m_render_resource->uploadGlobalRenderResource(m_rhi, level_resource_desc);

    // setup render camera
    const CameraPose& camera_pose = global_rendering_res.m_camera_config.m_pose;
    m_render_camera               = std::make_shared<RenderCamera>();
    m_render_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);
    m_render_camera->m_zfar  = global_rendering_res.m_camera_config.m_z_far;
    m_render_camera->m_znear = global_rendering_res.m_camera_config.m_z_near;
    m_render_camera->setAspect(global_rendering_res.m_camera_config.m_aspect.x /
                               global_rendering_res.m_camera_config.m_aspect.y);

    // setup render scene
    m_render_scene                  = std::make_shared<RenderScene>();
    m_render_scene->m_ambient_light = {global_rendering_res.m_ambient_light.toVector3()};
    m_render_scene->m_directional_light.m_direction =
        global_rendering_res.m_directional_light.m_direction.normalisedCopy();
    m_render_scene->m_directional_light.m_color = global_rendering_res.m_directional_light.m_color.toVector3();
    m_render_scene->setVisibleNodesReference();

    // initialize render pipeline
    FRenderPipelineInitInfo pipeline_init_info;
    pipeline_init_info.enable_fxaa     = global_rendering_res.m_enable_fxaa;
    pipeline_init_info.render_resource = m_render_resource;

    m_render_pipeline        = std::make_shared<URenderPipeline>();
    m_render_pipeline->m_rhi = m_rhi;
    m_render_pipeline->initialize(pipeline_init_info);

    // descriptor set layout in main camera pass will be used when uploading resource
    std::static_pointer_cast<RenderResource>(m_render_resource)->m_mesh_descriptor_set_layout =
        &static_cast<URenderPass*>(m_render_pipeline->m_main_camera_pass.get())
             ->m_descriptor_infos[UMainCameraPass::LayoutType::_per_mesh]
             .layout;
    std::static_pointer_cast<RenderResource>(m_render_resource)->m_material_descriptor_set_layout =
        &static_cast<URenderPass*>(m_render_pipeline->m_main_camera_pass.get())
             ->m_descriptor_infos[UMainCameraPass::LayoutType::_mesh_per_material]
             .layout;
}

void URenderSystem::tick(float delta_time)
{
    // process swap data between logic and render contexts
    processSwapData();

    // prepare render command context
    m_rhi->prepareContext();

    // update per-frame buffer
    m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

    // update per-frame visible objects
    m_render_scene->updateVisibleObjects(std::static_pointer_cast<RenderResource>(m_render_resource),
                                         m_render_camera);

    // prepare pipeline's render passes data
    m_render_pipeline->preparePassData(m_render_resource);

    g_runtime_global_context.m_debugdraw_manager->tick(delta_time);

    // 基类方法, 相机灯光阴影pass准备数据
    m_render_pipeline->deferredRender(m_rhi, m_render_resource);
}

void URenderSystem::clear()
{
    if (m_rhi)
    {
        m_rhi->clear();
    }
    m_rhi.reset();

    if (m_render_scene)
    {
        m_render_scene->clear();
    }
    m_render_scene.reset();

    if (m_render_resource)
    {
        m_render_resource->clear();
    }
    m_render_resource.reset();
    
    if (m_render_pipeline)
    {
        m_render_pipeline->clear();
    }
    m_render_pipeline.reset();
}

void URenderSystem::swapLogicRenderData() { m_swap_context.swapLogicRenderData(); }

RenderSwapContext& URenderSystem::getSwapContext() { return m_swap_context; }

std::shared_ptr<RenderCamera> URenderSystem::getRenderCamera() const { return m_render_camera; }

std::shared_ptr<RHI>          URenderSystem::getRHI() const { return m_rhi; }

void URenderSystem::updateEngineContentViewport(float offset_x, float offset_y, float width, float height)
{
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x        = offset_x;
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y        = offset_y;
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width    = width;
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height   = height;
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.minDepth = 0.0f;
    std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.maxDepth = 1.0f;

    m_render_camera->setAspect(width / height);
}

FEngineContentViewport URenderSystem::getEngineContentViewport() const
{
    float x      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.x;
    float y      = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.y;
    float width  = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.width;
    float height = std::static_pointer_cast<VulkanRHI>(m_rhi)->m_viewport.height;
    return {x, y, width, height};
}

uint32_t URenderSystem::getGuidOfPickedMesh(const Vector2& picked_uv)
{
    return m_render_pipeline->getGuidOfPickedMesh(picked_uv);
}

GObjectID URenderSystem::getGObjectIDByMeshID(uint32_t mesh_id) const
{
    return m_render_scene->getGObjectIDByMeshID(mesh_id);
}

void URenderSystem::createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas)
{
    for (int i = 0; i < axis_entities.size(); i++)
    {
        m_render_resource->uploadGameObjectRenderResource(m_rhi, axis_entities[i], mesh_datas[i]);
    }
}

void URenderSystem::setVisibleAxis(std::optional<RenderEntity> axis)
{
    m_render_scene->m_render_axis = axis;

    if (axis.has_value())
    {
        std::static_pointer_cast<URenderPipeline>(m_render_pipeline)->setAxisVisibleState(true);
    }
    else
    {
        std::static_pointer_cast<URenderPipeline>(m_render_pipeline)->setAxisVisibleState(false);
    }
}

void URenderSystem::setSelectedAxis(size_t selected_axis)
{
    std::static_pointer_cast<URenderPipeline>(m_render_pipeline)->setSelectedAxis(selected_axis);
}

GuidAllocator<GameObjectPartId>& URenderSystem::getGOInstanceIdAllocator()
{
    return m_render_scene->getInstanceIdAllocator();
}

GuidAllocator<MeshSourceDesc>& URenderSystem::getMeshAssetIdAllocator()
{
    return m_render_scene->getMeshAssetIdAllocator();
}

void URenderSystem::clearForLevelReloading()
{
    m_render_scene->clearForLevelReloading();

    ParticleSubmitRequest request;

    m_swap_context.getLogicSwapData().m_particle_submit_request = request;
}

void URenderSystem::initializeUIRenderBackend(WindowUI* window_ui)
{
    m_render_pipeline->initializeUIRenderBackend(window_ui);
}

void URenderSystem::processSwapData()
{
    RenderSwapData& swap_data = m_swap_context.getRenderSwapData();

    std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
    ASSERT(asset_manager);

    // TODO: update global resources if needed
    if (swap_data.m_level_resource_desc.has_value())
    {
        m_render_resource->uploadGlobalRenderResource(m_rhi, *swap_data.m_level_resource_desc);

        // reset level resource swap data to a clean state
        m_swap_context.resetLevelRsourceSwapData();
    }

    // update game object if needed
    if (swap_data.m_game_object_resource_desc.has_value())
    {
        while (!swap_data.m_game_object_resource_desc->isEmpty())
        {
            GameObjectDesc gobject = swap_data.m_game_object_resource_desc->getNextProcessObject();

            for (size_t part_index = 0; part_index < gobject.getObjectParts().size(); part_index++)
            {
                //获取gameobject的数据
                const auto&      game_object_part = gobject.getObjectParts()[part_index];
                GameObjectPartId part_id          = {gobject.getId(), part_index};

                bool is_entity_in_scene = m_render_scene->getInstanceIdAllocator().hasElement(part_id);

                //构造渲染用actor
                RenderEntity render_entity;
                render_entity.m_instance_id =
                    static_cast<uint32_t>(m_render_scene->getInstanceIdAllocator().allocGuid(part_id));
                render_entity.m_model_matrix = game_object_part.m_transform_desc.m_transform_matrix;

                m_render_scene->addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                // mesh properties
                MeshSourceDesc mesh_source    = {game_object_part.m_mesh_desc.m_mesh_file};
                bool           is_mesh_loaded = m_render_scene->getMeshAssetIdAllocator().hasElement(mesh_source);

                RenderMeshData mesh_data;
                if (!is_mesh_loaded)
                {
                    mesh_data = m_render_resource->loadMeshData(mesh_source, render_entity.m_bounding_box);
                }
                else
                {
                    render_entity.m_bounding_box = m_render_resource->getCachedBoudingBox(mesh_source);
                }

                render_entity.m_mesh_asset_id = m_render_scene->getMeshAssetIdAllocator().allocGuid(mesh_source);
                render_entity.m_enable_vertex_blending =
                    game_object_part.m_skeleton_animation_result.m_transforms.size() > 1; // take care
                render_entity.m_joint_matrices.resize(
                    game_object_part.m_skeleton_animation_result.m_transforms.size());
                for (size_t i = 0; i < game_object_part.m_skeleton_animation_result.m_transforms.size(); ++i)
                {
                    render_entity.m_joint_matrices[i] =
                        game_object_part.m_skeleton_animation_result.m_transforms[i].m_matrix;
                }

                // material properties
                MaterialSourceDesc material_source;
                if (game_object_part.m_material_desc.m_with_texture)
                {
                    material_source = {game_object_part.m_material_desc.m_base_color_texture_file,
                                       game_object_part.m_material_desc.m_metallic_roughness_texture_file,
                                       game_object_part.m_material_desc.m_normal_texture_file,
                                       game_object_part.m_material_desc.m_occlusion_texture_file,
                                       game_object_part.m_material_desc.m_emissive_texture_file};
                }
                else
                {
                    // TODO: move to default material definition json file
                    material_source = {
                        asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                        asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                        asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                        "",
                        ""};
                }
                bool is_material_loaded = m_render_scene->getMaterialAssetdAllocator().hasElement(material_source);

                RenderMaterialData material_data;
                if (!is_material_loaded)
                {
                    material_data = m_render_resource->loadMaterialData(material_source);
                }

                render_entity.m_material_asset_id =
                    m_render_scene->getMaterialAssetdAllocator().allocGuid(material_source);

                // create game object on the graphics api side
                if (!is_mesh_loaded)
                {
                    m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, mesh_data);
                }

                if (!is_material_loaded)
                {
                    m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, material_data);
                }

                // add object to render scene if needed
                if (!is_entity_in_scene)
                {
                    m_render_scene->m_render_entities.push_back(render_entity);
                }
                else
                {
                    for (auto& entity : m_render_scene->m_render_entities)
                    {
                        if (entity.m_instance_id == render_entity.m_instance_id)
                        {
                            entity = render_entity;
                            break;
                        }
                    }
                }
            }
            // after finished processing, pop this game object
            swap_data.m_game_object_resource_desc->pop();
        }

        // reset game object swap data to a clean state
        m_swap_context.resetGameObjectResourceSwapData();
    }

    // remove deleted objects
    if (swap_data.m_game_object_to_delete.has_value())
    {
        while (!swap_data.m_game_object_to_delete->isEmpty())
        {
            GameObjectDesc gobject = swap_data.m_game_object_to_delete->getNextProcessObject();
            m_render_scene->deleteEntityByGObjectID(gobject.getId());
            swap_data.m_game_object_to_delete->pop();
        }

        m_swap_context.resetGameObjectToDelete();
    }

    // process camera swap data
    if (swap_data.m_camera_swap_data.has_value())
    {
        if (swap_data.m_camera_swap_data->m_fov_x.has_value())
        {
            m_render_camera->setFOVx(*swap_data.m_camera_swap_data->m_fov_x);
        }

        if (swap_data.m_camera_swap_data->m_view_matrix.has_value())
        {
            m_render_camera->setMainViewMatrix(*swap_data.m_camera_swap_data->m_view_matrix);
        }

        if (swap_data.m_camera_swap_data->m_camera_type.has_value())
        {
            m_render_camera->setCurrentCameraType(*swap_data.m_camera_swap_data->m_camera_type);
        }

        m_swap_context.resetCameraSwapData();
    }

    if (swap_data.m_particle_submit_request.has_value())
    {
        std::shared_ptr<ParticlePass> particle_pass =
            std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass);

        int emitter_count = swap_data.m_particle_submit_request->getEmitterCount();
        particle_pass->setEmitterCount(emitter_count);

        for (int index = 0; index < emitter_count; ++index)
        {
            const ParticleEmitterDesc& desc = swap_data.m_particle_submit_request->getEmitterDesc(index);
            particle_pass->createEmitter(index, desc);
        }

        particle_pass->initializeEmitters();

        m_swap_context.resetPartilceBatchSwapData();
    }
    if (swap_data.m_emitter_tick_request.has_value())
    {
        std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
            ->setTickIndices(swap_data.m_emitter_tick_request->m_emitter_indices);
        m_swap_context.resetEmitterTickSwapData();
    }

    if (swap_data.m_emitter_transform_request.has_value())
    {
        std::static_pointer_cast<ParticlePass>(m_render_pipeline->m_particle_pass)
            ->setTransformIndices(swap_data.m_emitter_transform_request->m_transform_descs);
        m_swap_context.resetEmitterTransformSwapData();
    }
}
} // namespace Piccolo