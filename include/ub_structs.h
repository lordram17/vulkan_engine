#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MVPUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct LightUniformBufferObject {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 color;
	glm::vec3 ambient_color;
	glm::vec3 diffuse_color;
	glm::vec3 specular_color;
	float specular_power;
};