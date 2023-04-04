#include "runtime/function/controller/character_controller.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"

namespace Piccolo
{
    CharacterController::CharacterController(const Capsule& capsule) : m_capsule(capsule)
    {
        Quaternion orientation;
        orientation.fromAngleAxis(Radian(Degree(90.f)), Vector3::UNIT_X);
    }

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        Vector3 final_position = current_position + displacement;

        Transform final_transform = Transform(final_position, Quaternion::IDENTITY, Vector3::UNIT_SCALE);

        return final_position;
    }
} // namespace Piccolo
