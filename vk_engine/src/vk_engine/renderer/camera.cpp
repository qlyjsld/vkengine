#include "vk_engine/renderer/camera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace vk_engine {

	constexpr float camera_speed = 5.0f;

	void Camera::updateCameraFront(double mouse_xpos, double mouse_ypos) {
		float xoffset = (mouse_xpos - lastMouseXpos) * 0.1f;
		float yoffset = (lastMouseYpos - mouse_ypos) * 0.1f;

		yaw += xoffset;
		pitch += yoffset;

		camFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		camFront.y = sin(glm::radians(pitch));
		camFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		camFront = glm::normalize(camFront);

		lastMouseXpos = mouse_xpos;
		lastMouseYpos = mouse_ypos;
	}

	void  Camera::updateCameraPos(char&& key, float frametime) {
		switch (key) {
		case 'w':
			camPos += camFront * frametime * camera_speed;
			break;
		case 'a':
			camPos -= glm::cross(camFront, camUp) * frametime * camera_speed;
			break;
		case 's':
			camPos -= camFront * frametime * camera_speed;
			break;
		case 'd':
			camPos += glm::cross(camFront, camUp) * frametime * camera_speed;
			break;
		default:
			break;
		}
	}

	glm::mat4 Camera::getViewMatrix() {
		return glm::lookAt(camPos, camPos + camFront, camUp);
	}

	glm::mat4 Camera::getProjectionMatrix(float width, float height) {
		auto projection = glm::perspective(glm::radians(90.0f), width / height, 0.1f, 2000.0f);
		projection[1][1] *= -1;
		return projection;
	}

}