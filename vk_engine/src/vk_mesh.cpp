#include "vk_mesh.h"

VertexInputDescription Vertex::get_vertex_description() {
	VertexInputDescription description;

	// 1 vertex buffer binding
	VkVertexInputBindingDescription mainBinding{};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	// Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, position);

	// Position will be stored at Location 1
	VkVertexInputAttributeDescription colorAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 1;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, color);

	// Position will be stored at Location 2
	VkVertexInputAttributeDescription normalAttribute{};
	positionAttribute.binding = 0;
	positionAttribute.location = 2;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, normal);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(normalAttribute);

	return std::move(description);
}