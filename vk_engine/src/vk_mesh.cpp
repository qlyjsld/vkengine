#include "vk_mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <future>
#include <mutex>

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
	colorAttribute.binding = 0;
	colorAttribute.location = 1;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, color);

	// Position will be stored at Location 2
	VkVertexInputAttributeDescription normalAttribute{};
	normalAttribute.binding = 0;
	normalAttribute.location = 2;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Vertex, normal);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(normalAttribute);

	return description;
}

std::mutex vector_mutex;

void load_shape(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape, std::vector<Vertex>& vertices){
	size_t index_offset = 0;
	// loop over faces
	for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
		int fv = shape.mesh.num_face_vertices[f];
		// loop over vertices of face
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
			float vx = attrib.vertices[3 * idx.vertex_index + 0];
			float vy = attrib.vertices[3 * idx.vertex_index + 1];
			float vz = attrib.vertices[3 * idx.vertex_index + 2];
			float nx = attrib.normals[3 * idx.normal_index + 0];
			float ny = attrib.normals[3 * idx.normal_index + 1];
			float nz = attrib.normals[3 * idx.normal_index + 2];

			Vertex vertex;
			vertex.position.x = vx;
			vertex.position.y = vy;
			vertex.position.z = vz;

			vertex.normal.x = nx;
			vertex.normal.x = ny;
			vertex.normal.x = nz;

			vertex.color = vertex.position;

			std::lock_guard<std::mutex> lock(vector_mutex);
			vertices.push_back(std::move(vertex));
		}
		index_offset += fv;
	}
}

bool Mesh::load_from_obj(const char* filename) {
	// attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	// shapes contain the vertices of each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	// materials contain the material of each separate object in the file
	std::vector<tinyobj::material_t> materials;

	// err stands for error
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		std::async(std::launch::async, load_shape, attrib, shapes[s], std::ref(_vertices));
	}

	std::cout << "finished loading: " << filename << std::endl;

	return true;
}