#include "light.h"

glm::mat4 IVRLight::GetLightView()
{
	glm::vec3 world_up = glm::vec3(0, 1, 0);
	Direction = glm::normalize(Direction);
	glm::vec3 light_right = glm::cross(world_up, Direction);
	glm::vec3 light_up = glm::cross(Direction, light_right);

	glm::mat4 light_view = glm::lookAt(Position, Position + Direction, light_up);

	return light_view;
}

glm::mat4 IVRLight::GetLightProjection(float fov, float aspect_ratio, float near_plane, float far_plane)
{
	glm::mat4 light_projection = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
	light_projection[1][1] *= -1; //flip the y axis
	return light_projection;
}
