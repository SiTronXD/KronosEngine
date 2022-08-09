#pragma once

#include "../SMath.h"

class Camera
{
private:
	glm::mat4 view;
	glm::mat4 proj;

public:
	Camera();
	~Camera();

	inline const glm::mat4& const getViewMatrix() { return this->view; }
	inline const glm::mat4& const getProjectionMatrix() { return this->proj; }
};