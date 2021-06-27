#include "VkEngine/Renderer/SurfaceHandler.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr float WIDTH = 1600.0f;
constexpr float HEIGHT = 900.0f;

namespace VkEngine
{

    void SurfaceHandler::init(VkInstance instance)
    {
        if (!glfwInit())
			throw std::runtime_error("GLFW initialization failed!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(WIDTH, HEIGHT, "VkEngine", NULL, NULL);

        //create surface
		VK_CHECK(glfwCreateWindowSurface(instance, _window, nullptr, &_surface));

		_deletionQueue.push_function([=]()
		{
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
		});
    }

    void SurfaceHandler::release()
    {
        _deletionQueue.flush();

        glfwDestroyWindow(_window);
    }
}