#pragma once

#include <glm/glm.hpp>

enum class IVRLightType {
	Directional,
	Point
};

struct IVRLight {
	glm::vec3 Position;
	glm::vec3 Color;
	float Intensity;
	IVRLightType Type;
};