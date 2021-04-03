#include "Camera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

const float camera_speed = 5.0f;

void Camera::updateCameraFront(double mouse_xpos, double mouse_ypos) {
	float xoffset = (mouse_xpos - last_mouse_xpos) * 0.1f;
	float yoffset = (last_mouse_ypos - mouse_ypos) * 0.1f;

	yaw += xoffset;
	pitch += yoffset;
	
	camFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	camFront.y = sin(glm::radians(pitch));
	camFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	camFront = glm::normalize(camFront);

	last_mouse_xpos = mouse_xpos;
	last_mouse_ypos = mouse_ypos;
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
	view = glm::lookAt(camPos, camPos + camFront, camUp);
	lastFrame = currentFrame;
}

glm::mat4 Camera::modelViewMatrix() {
	return projection * view;
}