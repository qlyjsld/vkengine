#pragma once

#include "vk_engine/renderer/vk_type.h"
#include "vk_engine/renderer/vk_renderer.h"

namespace vk_engine {

	namespace vk_util {
		bool load_image_from_file(vk_engine::vk_renderer& engine, const char* file, AllocatedImage& outImage);
	}

}
