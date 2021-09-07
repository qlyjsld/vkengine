#include "VkEngine/Renderer/Texture.h"
#include "VkEngine/Renderer/Renderer.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Core/DeletionQueue.h"
#include "VkEngine/Asset/Asset.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace VkEngine
{

	bool loadImageFromFile(Renderer* renderer, const char* file, AllocatedImage& outImage)
	{
		Asset::AssetFile asset{};
		Asset::loadAssetFile(file, asset);

		Asset::TextureInfo textInfo = Asset::readTextureInfo(&asset);

		VkFormat image_format = (VkFormat) textInfo.format;


		BufferID stageingBuffer = BufferHandler::createBuffer(textInfo.textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		// copy texture data
		void* data;
		vmaMapMemory(BufferHandler::getAllocator(), BufferHandler::getBuffer(stageingBuffer)->_allocation, &data);

		std::cout << "compressed size: " << asset.binaryBlob.size() << std::endl;
		std::cout << "dest size: " << textInfo.textureSize << std::endl;

		Asset::unpackTexture(&textInfo, asset.binaryBlob.data(), asset.binaryBlob.size(), (char*) data);

		vmaUnmapMemory(BufferHandler::getAllocator(), BufferHandler::getBuffer(stageingBuffer)->_allocation);

		VkExtent3D imageExtent;
		imageExtent.width = textInfo.width;
		imageExtent.height = textInfo.height;
		imageExtent.depth = 1;

		VkImageCreateInfo imgInfo{};
		imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imgInfo.pNext = nullptr;

		imgInfo.imageType = VK_IMAGE_TYPE_2D;

		imgInfo.format = image_format;
		imgInfo.extent = imageExtent;

		imgInfo.mipLevels = 1;
		imgInfo.arrayLayers = 1;
		imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		AllocatedImage newImage;

		VmaAllocationCreateInfo img_allocInfo{};
		img_allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(BufferHandler::getAllocator(), &imgInfo, &img_allocInfo, &newImage._image, &newImage._allocation, nullptr);

		renderer->immediate_submit([&](VkCommandBuffer cmd)
		{
			VkImageSubresourceRange range{};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer{};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = newImage._image;
			imageBarrier_toTransfer.subresourceRange = range;
			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

			VkBufferImageCopy copyRegion{};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = imageExtent;

			vkCmdCopyBufferToImage(cmd, BufferHandler::getBuffer(stageingBuffer)->_buffer, newImage._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

		DeletionQueue::push_function([=]()
		{
			vmaDestroyImage(BufferHandler::getAllocator(), newImage._image, newImage._allocation);
		});

		vmaDestroyBuffer(BufferHandler::getAllocator(), BufferHandler::getBuffer(stageingBuffer)->_buffer, BufferHandler::getBuffer(stageingBuffer)->_allocation);

		outImage = newImage;

		return true;
	}
}