#pragma once

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

namespace VkEngine
{

	class Camera
	{
	public:

		// constructor
		Camera()
		{
			camPos = glm::vec3{ 0.0f, 0.0f, -5.0f };
			camFront = glm::vec3(0.0f, 0.0f, 1.0f);
			camUp = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		Camera(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camUp)
			: camPos(camPos), camFront(camFront), camUp(camUp)
		{}

		// destructor
		~Camera() {};

		// copy constructor
		Camera(const Camera& other)
		{
			camPos = other.camPos;
			camFront = other.camFront;
			camUp = other.camUp;
		}

		Camera& operator=(const Camera& other)
		{
			this->camPos = other.camPos;
			this->camFront = other.camFront;
			this->camUp = other.camUp;
			return *this;
		}

		// move constructor
		Camera(Camera&& other) noexcept
		{
			camPos = other.camPos;
			camFront = other.camFront;
			camUp = other.camUp;
		}

		Camera& operator=(Camera&& other) noexcept
		{
			if (this != &other) {
				this->camPos = other.camPos;
				this->camFront = other.camFront;
				this->camUp = other.camUp;
			}
			return *this;
		}

		glm::vec3 camPos;
		glm::vec3 camFront;
		glm::vec3 camUp;

		float yaw{ 0.0f };
		float pitch{ 0.0f };

		double lastMouseXpos{ 0.0 };
		double lastMouseYpos{ 0.0 };

		void updateCameraFront(double mouse_xpos, double mouse_ypos);
		void updateCameraPos(char&& key, float frametime);

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix(float width, float height);
	};
}