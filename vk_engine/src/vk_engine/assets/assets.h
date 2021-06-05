#pragma once
#include <string>
#include <vector>

namespace vk_engine {

    namespace assets {

        struct assestFile {
            char type[4]; // TEXT for texture, MESH for mesh
            uint32_t version;
            std::string json;
            std::vector<char> binaryBlob;
        };

        bool saveBinaryFile(const char* path, const assestFile& file);
        bool loadBinaryFile(const char* path, assestFile& file);

        // texture
        struct textureInfo {
            uint32_t textureSize;
            uint32_t pixelSize[3];
            std::string pixels;
        }

        textureInfo readTextureInfo(assestFile* file);
        void unpackTexture(textureInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest);
        assestFile packTexture(textureInfo* info, void* pixelData);

        // mesh

    }

}