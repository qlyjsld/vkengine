#include "vk_engine/renderer/vk_texture.h"
#include "vk_engine/renderer/vk_info.h"
#include "vk_engine/assets/assets.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace vk_engine {

	bool vk_util::load_image_from_file(vk_renderer& engine, const char* file, AllocatedImage& outImage) {
		assets::assetFile asset{};
		assets::loadAssetFile(file, asset);

		assets::textureInfo textInfo = assets::readTextureInfo(&asset);

		VkFormat image_format = (VkFormat) textInfo.format;

		AllocatedBuffer stageingBuffer = engine.create_buffer(textInfo.textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		// copy texture data
		void* data;
		vmaMapMemory(engine._allocator, stageingBuffer._allocation, &data);

		assets::unpackTexture(&textInfo, asset.binaryBlob.data(), asset.binaryBlob.size(), (char*) data);

		vmaUnmapMemory(engine._allocator, stageingBuffer._allocation);

		VkExtent3D imageExtent;
		imageExtent.width = textInfo.width;
		imageExtent.height = textInfo.height;
		imageExtent.depth = 1;

		VkImageCreateInfo imgInfo = vk_info::ImageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

		AllocatedImage newImage;

		VmaAllocationCreateInfo img_allocInfo{};
		img_allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(engine._allocator, &imgInfo, &img_allocInfo, &newImage._image, &newImage._allocation, nullptr);

		engine.immediate_submit([&](VkCommandBuffer cmd) {
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

			vkCmdCopyBufferToImage(cmd, stageingBuffer._buffer, newImage._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
			});

		engine._deletionQueue.push_function([=]() {
			vmaDestroyImage(engine._allocator, newImage._image, newImage._allocation);
			});

		vmaDestroyBuffer(engine._allocator, stageingBuffer._buffer, stageingBuffer._allocation);

		outImage = newImage;

		return true;
	}

}