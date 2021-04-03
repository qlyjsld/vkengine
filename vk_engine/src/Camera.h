#pragma once
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
	// constructor
	Camera(float width, float height) {
		camPos = glm::vec3{ 0.0f, 0.0f, -5.0f };
		camFront = glm::vec3(0.0f, 0.0f, 1.0f);
		camUp = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(camPos, camPos + camFront, camUp);
		projection = glm::perspective(glm::radians(70.0f), width / height, 0.1f, 200.0f);
		projection[1][1] *= -1;
	}

	Camera(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camUp, const glm::mat4& view, const glm::mat4& projection)
	: camPos(camPos), camFront(camFront), camUp(camUp), view(view), projection(projection)
	{}

	// destructor
	~Camera() { release(); };

	// copy constructor
	Camera(const Camera& other) {
		camPos = other.camPos;
		camFront = other.camFront;
		camUp = other.camUp;
		view = other.view;
		projection = other.projection;
	}

	Camera& operator=(const Camera& other) {
		this->camPos = other.camPos;
		this->camFront = other.camFront;
		this->camUp = other.camUp;
		this->view = other.view;
		this->projection = other.projection;
		return *this;
	}

	// move constructor
	Camera(Camera&& other) noexcept {
		camPos = other.camPos;
		camFront = other.camFront;
		camUp = other.camUp;
		view = other.view;
		projection = other.projection;
	}

	Camera& operator=(Camera&& other) noexcept {
		if (this != &other) {
			this->camPos = other.camPos;
			this->camFront = other.camFront;
			this->camUp = other.camUp;
			this->view = other.view;
			this->projection = other.projection;
			other.release();
		}
		return *this;
	}

	glm::vec3 camPos;
	glm::vec3 camFront;
	glm::vec3 camUp;

	glm::mat4 view;
	glm::mat4 projection;

	void updateCameraFront(double mouse_xpos, double mouse_ypos);
	void updateCameraPos(char&& key, float frametime);
	glm::mat4 modelViewMatrix();

private:
	double last_mouse_xpos{ 0.0 };
	double last_mouse_ypos{ 0.0 };

	float currentFrame{ 0.0f };
	float lastFrame{ 0.0f };

	float yaw{ 0.0f };
	float pitch{ 0.0f };

	void release() {
		// release resources
	}
};