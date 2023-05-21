#pragma once
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

class IVRCamera {

private:
    //camera specifications
    glm::vec3 CameraPosition_{};
    glm::vec3 CameraDirection_{};
    glm::vec3 CameraRight_{};
    glm::vec3 CameraUp_{};
    glm::vec3 WorldUp_ = glm::vec3(0,1,0);

    float Pitch = 0.0f;
    float Yaw = 0.0f;

    glm::mat4 PreviousView;
    int PreviousMouseX = 0;
    int PreviousMouseY = 0;

public:

    IVRCamera();

    float FieldOfView = 90.0f;
    float AspectRatio = 1.77f;
    float NearPlane = 0.1f;
    float FarPlane = 100.0f;

    float CameraMoveSpeed = 10.0f;
    float CameraTurnSpeed = 2.0f;


    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();
    
    void SetPosition(glm::vec3 position);

    void MoveCamera(float dt);

};