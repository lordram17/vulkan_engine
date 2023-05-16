#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct IVRTransform {
	
	glm::vec3 Position = glm::vec3(0,0,0);
	glm::vec3 Rotation = glm::vec3(0, 0, 0);
	glm::vec3 Scale = glm::vec3(1, 1, 1);

	//matrix to convert from model space to world space
	glm::mat4 GetModelMatrix()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, Position);
		model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, Scale);
		return model;
	}
};
