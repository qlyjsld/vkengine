#pragma once
#include <string>
#include <vector>

namespace vk_engine {

    namespace assets {

        struct assetFile {
            char type[4]; // TEXT for texture, MESH for mesh
            uint32_t version;
            std::string json;
            std::vector<char> binaryBlob;
        };

        bool saveAssetFile(const char* path, const assetFile& file);
        bool loadAssetFile(const char* path, assetFile& file);

        // texture
        enum class textureFormat : uint32_t {
            UNDEFINED = 0,
            RGBA8 = 43
        };

        struct textureInfo {
            uint64_t textureSize;
            textureFormat format;
            uint32_t width;
            uint32_t height;
        };

        // textureFormat parseFormat(const char* c);
        textureInfo readTextureInfo(assetFile* file);
        void unpackTexture(textureInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest);
        assetFile packTexture(textureInfo* info, void* pixelData);

        // mesh
        struct meshInfo {

        };

        /* meshInfo readMeshInfo(assetFile* file);
        void unpackMesh(meshInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest);
        assetFile packMesh(meshInfo* info, void* pixelData); */

    }

}