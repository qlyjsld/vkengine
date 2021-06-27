#include "VkEngine/Renderer/Mesh.h"
#include "VkEngine/Asset/Asset.h"
#include "VkEngine/Renderer/Renderer.h"

#include <iostream>
#include <future>
#include <mutex>

namespace VkEngine
{

	VertexInputDescription Vertex::getVertexDescription()
	{
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

	void Mesh::loadFromObj(const char* filename, vk_renderer* renderer)
	{
		assets::assetFile asset{};
		assets::loadAssetFile(filename, asset);

		assets::meshInfo info = assets::readMeshInfo(&asset);

		// allocate Staging Buffer
		VkBufferCreateInfo stagingBufferInfo{};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// total size in bytes
		stagingBufferInfo.size = info.meshSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// let vma know this buffer is gonna written by cpu and read by gpu
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		AllocatedBuffer stagingBuffer;

		vmaCreateBuffer(renderer->_allocator, &stagingBufferInfo, &allocationInfo, &stagingBuffer._buffer, &stagingBuffer._allocation, nullptr);

		// copy vertex data
		void* data;
		vmaMapMemory(renderer->_allocator, stagingBuffer._allocation, &data);

		std::cout << "compressed size: " << asset.binaryBlob.size() << std::endl;
		std::cout << "dest size: " << info.meshSize << std::endl;

		assets::unpackMesh(&info, asset.binaryBlob.data(), asset.binaryBlob.size(), (char*) data);

		std::cout << "unpacked!" << std::endl;

		vmaUnmapMemory(renderer->_allocator, stagingBuffer._allocation);

		// allocate Vertex Buffer
		VkBufferCreateInfo vertexBufferInfo{};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// total size in bytes
		vertexBufferInfo.size = asset.binaryBlob.size();
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// let vma know this buffer is gonna written by cpu and read by gpu
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		Mesh mesh;
		mesh._vertices.resize(info.meshSize / sizeof(Vertex));

		vmaCreateBuffer(renderer->_allocator, &vertexBufferInfo, &allocationInfo, &mesh._vertexBuffer._buffer, &mesh._vertexBuffer._allocation, nullptr);

		renderer->_deletionQueue.push_function([=]()
		{
			vmaDestroyBuffer(renderer->_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
		});

		renderer->immediate_submit([=](VkCommandBuffer cmd)
		{
			VkBufferCopy copy;
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = asset.binaryBlob.size();
			vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh._vertexBuffer._buffer, 1, &copy);
		});

		vmaDestroyBuffer(renderer->_allocator, stagingBuffer._buffer, stagingBuffer._allocation);

		renderer->_meshes[filename] = std::move(mesh);

		std::cout << "finished loading: " << filename << std::endl;
	}
}