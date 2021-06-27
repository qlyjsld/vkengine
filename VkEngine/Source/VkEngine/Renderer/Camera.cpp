#include "VkEngine/Renderer/Camera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace VkEngine
{

	constexpr float cameraSpeed = 5.0f;

	void Camera::updateCameraFront(double mouseXpos, double mouseYpos)
	{
		float xoffset = (mouseXpos - lastMouseXpos) * 0.1f;
		float yoffset = (lastMouseYpos - mouseYpos) * 0.1f;

		yaw += xoffset;
		pitch += yoffset;

		camFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		camFront.y = sin(glm::radians(pitch));
		camFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		camFront = glm::normalize(camFront);

		lastMouseXpos = mouseXpos;
		lastMouseYpos = mouseYpos;
	}

	void  Camera::updateCameraPos(char&& key, float frametime)
	{
		switch (key) {
		case 'w':
			camPos += camFront * frametime * cameraSpeed;
			break;
		case 'a':
			camPos -= glm::cross(camFront, camUp) * frametime * cameraSpeed;
			break;
		case 's':
			camPos -= camFront * frametime * cameraSpeed;
			break;
		case 'd':
			camPos += glm::cross(camFront, camUp) * frametime * cameraSpeed;
			break;
		default:
			break;
		}
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(camPos, camPos + camFront, camUp);
	}

	glm::mat4 Camera::getProjectionMatrix(float width, float height)
	{
		auto projection = glm::perspective(glm::radians(90.0f), width / height, 0.1f, 2000.0f);
		projection[1][1] *= -1;
		return projection;
	}
}