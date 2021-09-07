#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"
#include "VkEngine/Core/ConsoleVariableSystem.h"
#include "VkEngine/Core/GlobalMacro.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

constexpr float WIDTH = 1600.0f;
constexpr float HEIGHT = 900.0f;

namespace VkEngine
{

    SurfaceHandler::SurfaceHandler(VkInstance instance)
    {
        if (!glfwInit())
			throw std::runtime_error("GLFW initialization failed!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(WIDTH, HEIGHT, "VkEngine", NULL, NULL);

        //create surface
		VK_CHECK(glfwCreateWindowSurface(instance, _window, nullptr, &_surface));

		DeletionQueue::push_function([=]()
		{
			vkDestroySurfaceKHR(instance, _surface, nullptr);
		});
    }

    SurfaceHandler::~SurfaceHandler()
    {
        glfwDestroyWindow(_window);
    }
}