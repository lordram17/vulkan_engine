#include "camera.h"
#include "input_manager.h"
#include "debug_logger_utils.h"

glm::mat4 IVRCamera::GetViewMatrix()
{

    //construct the view matrix
    glm::mat4 view = glm::mat4(1.0f); 
    view = glm::lookAt(CameraPosition_, CameraPosition_ + CameraDirection_, CameraUp_);

    return view;
}

glm::mat4 IVRCamera::GetProjectionMatrix()
{
    glm::mat4 projection_matrix = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);
    return projection_matrix;
}

void IVRCamera::SetPosition(glm::vec3 position)
{
    CameraPosition_ = position;
}

void IVRCamera::MoveCamera(float dt)
{
    //direction calculation
    float delta_horizontal_angle = CameraTurnSpeed * dt * IVRMouseStatus::MouseX_;
    float delta_vertical_angle = CameraTurnSpeed * dt * IVRMouseStatus::MouseY_;

    IVR_LOG_INFO(IVRMouseStatus::MouseX_);

    HorizontalAngle += delta_horizontal_angle;
    VerticalAngle -= delta_vertical_angle;

    CameraDirection_ = glm::vec3(
		cos(VerticalAngle) * sin(HorizontalAngle),
		sin(VerticalAngle),
		cos(VerticalAngle) * cos(HorizontalAngle)
	);

    CameraRight_ = glm::vec3(
        sin(HorizontalAngle - 3.14f / 2.0f),
        0,
        cos(HorizontalAngle - 3.14f / 2.0f)
    );

    CameraUp_ = glm::cross(CameraRight_, CameraDirection_);

    if (IVRKeyStatus::GetKeyStatus(IVRKey::A) || IVRKeyStatus::GetKeyStatus(IVRKey::LEFT)) CameraPosition_ -= CameraRight_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::D) || IVRKeyStatus::GetKeyStatus(IVRKey::RIGHT)) CameraPosition_ += CameraRight_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::W) || IVRKeyStatus::GetKeyStatus(IVRKey::UP)) CameraPosition_ += CameraDirection_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::S) || IVRKeyStatus::GetKeyStatus(IVRKey::DOWN)) CameraPosition_ -= CameraDirection_ * CameraMoveSpeed * dt;
}


