#pragma once

#include <string>
#include <vector>

namespace VkEngine
{

    namespace Asset
    {

        struct AssetFile
        {
            char type[4]; // TEXT for texture, MESH for mesh
            uint32_t version;
            std::string json;
            std::vector<char> binaryBlob;
        };

        bool saveAssetFile(const char* path, const AssetFile& file);
        bool loadAssetFile(const char* path, AssetFile& file);

        // texture
        enum class TextureFormat : uint32_t
        {
            UNDEFINED = 0,
            RGBA8 = 43
        };

        struct TextureInfo
        {
            uint64_t textureSize;
            TextureFormat format;
            uint32_t width;
            uint32_t height;
        };

        // textureFormat parseFormat(const char* c);
        TextureInfo readTextureInfo(AssetFile* file);
        void unpackTexture(TextureInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest);
        AssetFile packTexture(TextureInfo* info, void* pixelData);

        // mesh
        struct Vertex
        {
            float position[3];
            float normal[3];
            float color[3];
            float uv[2];
        };

        struct Mesh
        {
            std::vector<Vertex> _vertices;
        };

        struct MeshInfo
        {
            uint32_t shapeSize;
            uint64_t meshSize;
        };

        MeshInfo readMeshInfo(AssetFile* file);
        void unpackMesh(MeshInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest);
        AssetFile packMesh(MeshInfo* info, void* meshData);
    }

}