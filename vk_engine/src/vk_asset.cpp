#include <string>
#include <iostream>
#include "vk_engine/assets/assets.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int main(int argc, char** argv) {
	// texture
	/*
	for (int i = 1; i < argc; i++) {
		std::string filePath = argv[i];

		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			return false;
		}

		vk_engine::assets::textureInfo info{};
		info.width = texWidth;
		info.height = texHeight;
		info.textureSize = texWidth * texHeight * 4;
		info.format = vk_engine::assets::textureFormat::RGBA8;

		void* pixel_ptr = pixels;

		vk_engine::assets::assetFile file = vk_engine::assets::packTexture(&info, pixel_ptr);

		std::cout << filePath.substr(0, filePath.size() - 4) + ".asset" << std::endl;

		vk_engine::assets::saveAssetFile((filePath.substr(0, filePath.size() - 4) + ".asset").c_str(), file);
	}

	return 0;

	*/

    std::string filePath = argv[1];

	// mesh
    // attrib will contain the vertex arrays of the file
    tinyobj::attrib_t attrib;
    // shapes contain the vertices of each separate object in the file
    std::vector<tinyobj::shape_t> shapes;
    // materials contain the material of each separate object in the file
    std::vector<tinyobj::material_t> materials;

    // err stands for error
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str());

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        return 0;
    }

    std::vector<vk_engine::assets::Mesh> meshes;

    for (size_t s = 0; s < shapes.size(); s++) {
        vk_engine::assets::Mesh mesh;

        size_t index_offset = 0;
        // loop over faces
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            // loop over vertices of face
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];
                float nx = attrib.normals[3 * idx.normal_index + 0];
                float ny = attrib.normals[3 * idx.normal_index + 1];
                float nz = attrib.normals[3 * idx.normal_index + 2];

                vk_engine::assets::Vertex vertex;
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

                mesh._vertices.push_back(std::move(vertex));
            }
            index_offset += fv;
        }

        meshes.push_back(std::move(mesh));
    }

    vk_engine::assets::meshInfo info{};
    info.meshSize = meshes.size() * sizeof(vk_engine::assets::Vertex);
    info.shapeSize = shapes.size();

    void* meshPtr = meshes.data();
    vk_engine::assets::assetFile file = vk_engine::assets::packMesh(&info, meshPtr);

    std::cout << filePath.substr(0, filePath.size() - 4) + ".asset" << std::endl;

    vk_engine::assets::saveAssetFile((filePath.substr(0, filePath.size() - 4) + ".asset").c_str(), file);

    return 0;
}