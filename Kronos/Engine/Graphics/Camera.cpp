#include "Camera.h"

Camera::Camera()
	: view(glm::mat4(1.0f)),
	proj(glm::mat4(1.0f))
{
	this->view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	this->proj = glm::perspective(
		glm::radians(45.0f),
		16.0f / 9.0f,
		0.1f,
		100.0f
	);
}

Camera::~Camera()
{
}
