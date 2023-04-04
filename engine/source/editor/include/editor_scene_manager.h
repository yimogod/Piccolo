#pragma once

#include "editor/include/axis.h"

#include "runtime/function/framework/object/object.h"
#include "runtime/function/render/render_object.h"

#include <memory>

namespace Piccolo
{
    class PiccoloEditor;
    class RenderCamera;
    class RenderEntity;

    class EditorSceneManager
    {
    public:
        void initialize();
        void tick(float delta_time);

    public:
        std::weak_ptr<GObject> getSelectedGObject() const;
        void onGObjectSelected(GObjectID selected_gobject_id);
        void onDeleteSelectedGObject();
        void setEditorCamera(std::shared_ptr<RenderCamera> camera) { m_camera = camera; }

    public:
        std::shared_ptr<RenderCamera> getEditorCamera() { return m_camera; };

        GObjectID getSelectedObjectID() { return m_selected_gobject_id; };
        Matrix4x4 getSelectedObjectMatrix() { return m_selected_object_matrix; }

        void setSelectedObjectID(GObjectID selected_gobject_id) { m_selected_gobject_id = selected_gobject_id; };
        void setSelectedObjectMatrix(Matrix4x4 new_object_matrix) { m_selected_object_matrix = new_object_matrix; }
    private:
        GObjectID m_selected_gobject_id{ k_invalid_gobject_id };
        Matrix4x4 m_selected_object_matrix{ Matrix4x4::IDENTITY };
        std::shared_ptr<RenderCamera> m_camera;

        bool   m_is_show_axis = true;
    };
}