#include "Camera.h"
#include "Renderer.h"

void Camera::updateMatrices()
{
	this->viewMatrix = glm::lookAt(
		this->position,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	this->projectionMatrix = glm::perspective(
		glm::radians(45.0f),
		this->renderer.getSwapchainAspectRatio(),
		0.1f,
		100.0f
	);
}

Camera::Camera(Renderer& renderer)
	: renderer(renderer),
	viewMatrix(glm::mat4(1.0f)),
	projectionMatrix(glm::mat4(1.0f)),

	position(2.0f, 2.0f, 2.0f)
{
	this->updateMatrices();
}

Camera::~Camera()
{
}

void Camera::update()
{


	this->updateMatrices();
}
