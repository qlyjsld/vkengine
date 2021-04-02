#include "Camera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void Camera::updateCameraFront(double mouse_xpos, double mouse_ypos) {
	float yaw = (mouse_xpos - last_mouse_xpos) * 0.1f;
	float pitch = (last_mouse_ypos - mouse_ypos) * 0.1f;
	
	camFront.x = camFront.x * cos(glm::radians(yaw)) - camFront.z * sin(glm::radians(yaw));
	camFront.z = camFront.x * sin(glm::radians(yaw)) + camFront.z * cos(glm::radians(yaw));

	camFront.z = camFront.z * cos(glm::radians(pitch)) - camFront.y * sin(glm::radians(pitch));
	camFront.y = camFront.z * sin(glm::radians(pitch)) + camFront.y * cos(glm::radians(pitch));

	last_mouse_xpos = mouse_xpos;
	last_mouse_ypos = mouse_ypos;
}

void  Camera::updateCameraPos(char&& key, float frametime) {
	switch (key) {
	case 'w':
		camPos += camFront * frametime, 1.0f;
		break;
	case 'a':
		camPos -= glm::cross(camFront, camUp) * frametime, 1.0f;
		break;
	case 's':
		camPos -= camFront * frametime, 1.0f;
		break;
	case 'd':
		camPos += glm::cross(camFront, camUp) * frametime, 1.0f;
		break;
	default:
		break;
	}
	view = glm::lookAt(camPos, camPos + camFront, camUp);
	lastFrame = currentFrame;
}

glm::mat4 Camera::modelViewMatrix() {
	return projection * view;
}