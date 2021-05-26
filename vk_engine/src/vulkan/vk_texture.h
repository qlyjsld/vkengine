#pragma once

#include "vk_type.h"
#include "vk_engine.h"

namespace vk_util {
	bool load_image_from_file(vk_engine& engine, const char* file, AllocatedImage& outImage);
}
