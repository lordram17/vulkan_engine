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
    projection_matrix[1][1] *= -1; //flip the y axis
    return projection_matrix;
}

IVRCamera::IVRCamera()
{
	IVR_LOG_INFO("IVRCamera constructor called");
    IVR_LOG_INFO("Yaw: " + std::to_string(Yaw));
}

void IVRCamera::SetPosition(glm::vec3 position)
{
    CameraPosition_ = position;
}

void IVRCamera::MoveCamera(float dt)
{
    int XOffset = IVRMouseStatus::MouseX_ - PreviousMouseX;
    int YOffset = IVRMouseStatus::MouseY_ - PreviousMouseY;
    PreviousMouseX = IVRMouseStatus::MouseX_;
    PreviousMouseY = IVRMouseStatus::MouseY_;

    //direction calculation
    float yaw_offset = CameraTurnSpeed * dt * XOffset;
    float pitch_offset = CameraTurnSpeed * dt * YOffset;

    Yaw += yaw_offset;
    Pitch -= pitch_offset;

    if(Pitch > 89.0f) Pitch = 89.0f;
    if(Pitch < -89.0f) Pitch = -89.0f;

    CameraDirection_ = glm::vec3(
		cos(glm::radians(Pitch)) * cos(glm::radians(Yaw)),
		sin(glm::radians(Pitch)),
		cos(glm::radians(Pitch)) * sin(glm::radians(Yaw))
	);
    glm::normalize(CameraDirection_);

    //CameraRight_ = glm::vec3(
    //    sin(Yaw - 3.14f / 2.0f),
    //    0,
    //    cos(Yaw - 3.14f / 2.0f)
    //);

    CameraRight_ = glm::cross(CameraDirection_, WorldUp_);
    CameraUp_ = glm::cross(CameraRight_, CameraDirection_);

    if (IVRKeyStatus::GetKeyStatus(IVRKey::A) || IVRKeyStatus::GetKeyStatus(IVRKey::LEFT)) CameraPosition_ -= CameraRight_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::D) || IVRKeyStatus::GetKeyStatus(IVRKey::RIGHT)) CameraPosition_ += CameraRight_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::W) || IVRKeyStatus::GetKeyStatus(IVRKey::UP)) CameraPosition_ += CameraDirection_ * CameraMoveSpeed * dt;
    if (IVRKeyStatus::GetKeyStatus(IVRKey::S) || IVRKeyStatus::GetKeyStatus(IVRKey::DOWN)) CameraPosition_ -= CameraDirection_ * CameraMoveSpeed * dt;
}


