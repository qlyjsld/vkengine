#include "vk_engine/renderer/vk_mesh.h"
#include "vk_engine/assets/assets.h"

#include <iostream>
#include <future>
#include <mutex>

namespace vk_engine {

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

		// Position will be stored at Location 3
		VkVertexInputAttributeDescription uvAttribute{};
		uvAttribute.binding = 0;
		uvAttribute.location = 3;
		uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		uvAttribute.offset = offsetof(Vertex, uv);

		description.attributes.push_back(positionAttribute);
		description.attributes.push_back(colorAttribute);
		description.attributes.push_back(normalAttribute);
		description.attributes.push_back(uvAttribute);

		return description;
	}

	std::mutex vector_mutex;

	Mesh load_shape(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
		Mesh mesh;

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

				vertex.color = vertex.normal;

				// vertex uv
				tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

				vertex.uv.x = ux;
				vertex.uv.y = 1 - uy;

				std::lock_guard<std::mutex> lock(vector_mutex);
				mesh._vertices.push_back(std::move(vertex));
			}
			index_offset += fv;
		}

		return mesh;
	}

	std::unordered_map<std::string, Mesh> Mesh::load_from_obj(const char* filename) {
		std::unordered_map<std::string, Mesh> meshes;

		assets::assetFile file;
		assets::loadAssetFile(filename, file);

		assets::meshInfo info = assets::readMeshInfo(&file);

		// Loop over shapes
		for (size_t s = 0; s < info.shapeSize; s++) {
			meshs.push_back();
		}

		std::cout << "finished loading: " << filename << std::endl;

		return meshes;
	}

}