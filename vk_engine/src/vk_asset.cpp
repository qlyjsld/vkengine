#include "vk_engine/assets/assets.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {

	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load("assets/lost_empire-RGBA.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

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

	vk_engine::assets::saveAssetFile("assets/lost_empire-RGBA.asset", file);

	return 0;
}