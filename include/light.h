#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class IVRLightType {
	Directional,
	Point,
	Spot
};

//the light struct which stores the light from the perspective of the engine
struct IVRLight {
	glm::vec3 Position;
	glm::vec3 Direction;
	glm::vec3 AmbientColor;
	glm::vec3 DiffuseColor;
	glm::vec3 SpecularColor;
	IVRLightType Type;

	glm::mat4 GetLightView();
	glm::mat4 GetLightProjection(float fov, float aspect_ratio, float near_plane, float far_plane);
};

//the light struct that is passed to the shader (position/direction are transformed by the view matrix)
struct LightUBObj {
	alignas(16) glm::vec3 Position;
	alignas(16) glm::vec3 Direction;
	alignas(16) glm::vec3 AmbientColor;
	alignas(16) glm::vec3 DiffuseColor;
	alignas(16) glm::vec3 SpecularColor;

	void operator=(const IVRLight& light)
	{
		Position = light.Position;
		Direction = light.Direction;
		AmbientColor = light.AmbientColor;
		DiffuseColor = light.DiffuseColor;
		SpecularColor = light.SpecularColor;
	}

	LightUBObj(const IVRLight& light)
	{
		*this = light;
	}
};