#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MVPUBObj {
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Proj;
};

struct MaterialPropertiesUBObj {
	float SpecularPower = 0;
	uint32_t IsCubemap = 0;
	alignas (16) glm::vec3 SpecularColor = glm::vec3(0, 0, 0);
	alignas (16) glm::vec3 DiffuseColor = glm::vec3(0, 0, 0);
};

