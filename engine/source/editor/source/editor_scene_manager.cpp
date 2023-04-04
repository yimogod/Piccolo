#include <cassert>
#include <mutex>

#include "editor/include/editor.h"
#include "editor/include/editor_global_context.h"
#include "editor/include/editor_scene_manager.h"

#include "runtime/core/base/macro.h"

#include "runtime/engine.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/Renderer/RenderSystem.h"

namespace Piccolo
{
    void EditorSceneManager::initialize() {}

    void EditorSceneManager::tick(float delta_time)
    {
        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            TransformComponent* transform_component = selected_gobject->tryGetComponent(TransformComponent);
            if (transform_component)
            {
                transform_component->setDirtyFlag(true);
            }
        }
    }

    float intersectPlaneRay(Vector3 normal, float d, Vector3 origin, Vector3 dir)
    {
        float deno = normal.dotProduct(dir);
        if (fabs(deno) < 0.0001)
        {
            deno = 0.0001;
        }

        return -(normal.dotProduct(origin) + d) / deno;
    }

    std::weak_ptr<GObject> EditorSceneManager::getSelectedGObject() const
    {
        std::weak_ptr<GObject> selected_object;
        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            std::shared_ptr<Level> level = g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (level != nullptr)
            {
                selected_object = level->getGObjectByID(m_selected_gobject_id);
            }
        }
        return selected_object;
    }

    void EditorSceneManager::onGObjectSelected(GObjectID selected_gobject_id)
    {
        if (selected_gobject_id == m_selected_gobject_id)
            return;

        m_selected_gobject_id = selected_gobject_id;

        std::shared_ptr<GObject> selected_gobject = getSelectedGObject().lock();
        if (selected_gobject)
        {
            const TransformComponent* transform_component = selected_gobject->tryGetComponentConst(TransformComponent);
            m_selected_object_matrix                      = transform_component->getMatrix();
        }

        if (m_selected_gobject_id != k_invalid_gobject_id)
        {
            LOG_INFO("select game object " + std::to_string(m_selected_gobject_id));
        }
        else
        {
            LOG_INFO("no game object selected");
        }
    }

    void EditorSceneManager::onDeleteSelectedGObject()
    {
        // delete selected entity
        std::shared_ptr<GObject> selected_object = getSelectedGObject().lock();
        if (selected_object != nullptr)
        {
            std::shared_ptr<Level> current_active_level =
                g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
            if (current_active_level == nullptr)
                return;

            current_active_level->deleteGObjectByID(m_selected_gobject_id);

            RenderSwapContext& swap_context = g_editor_global_context.m_render_system->getSwapContext();
            swap_context.getLogicSwapData().addDeleteGameObject(GameObjectDesc {selected_object->getID(), {}});
        }
        onGObjectSelected(k_invalid_gobject_id);
    }
} // namespace Piccolo
