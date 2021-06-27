#pragma once

#include "VkEngine/Renderer/vk_type.h"
#include "VkEngine/Renderer/Renderer.h"

namespace VkEngine
{

	namespace VkUtil
	{

		bool loadImageFromFile(VkEngine::Renderer* renderer, const char* file, AllocatedImage& outImage);
	}
}
