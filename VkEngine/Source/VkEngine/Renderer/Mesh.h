#pragma once

// #include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>

namespace VkEngine
{

	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static VertexInputDescription getVertexDescription();
	};

	struct Mesh
	{
		std::vector<Vertex> _vertices;
		// glm::mat4 transformMatrix;

		AllocatedBuffer _vertexBuffer;
		static void loadFromObj(const char* filename, struct Renderer* renderer);
	};

}