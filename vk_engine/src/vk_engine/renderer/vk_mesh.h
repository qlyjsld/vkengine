#pragma once
#include "vk_engine/renderer/vk_type.h"
// #include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <string>
#include <unordered_map>

namespace vk_engine {

	struct VertexInputDescription {
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static VertexInputDescription get_vertex_description();
	};

	struct Mesh {
		std::vector<Vertex> _vertices;
		// glm::mat4 transformMatrix;

		AllocatedBuffer _vertexBuffer;
		static void load_from_obj(const char* filename, struct vk_renderer* renderer);
	};

}