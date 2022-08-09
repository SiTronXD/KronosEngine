#pragma once

#include "../SMath.h"

class Renderer;

class Camera
{
private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 position;

	Renderer& renderer;

	void updateMatrices();

public:
	Camera(Renderer& renderer);
	~Camera();

	void update();

	inline const glm::mat4& getViewMatrix() const { return this->viewMatrix; }
	inline const glm::mat4& getProjectionMatrix() const { return this->projectionMatrix; }
};