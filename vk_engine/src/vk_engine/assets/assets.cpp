#include "vk_engine/assets/assets.h"
#include "json.hpp"
#include "lz4.h"
#include <fstream>

using json = nlohmann::json;

namespace vk_engine {

    namespace assets {

        bool saveAssetFile(const char* path, const assetFile& file) {
            std::ofstream binaryFile;
            binaryFile.open(path, std::ios::binary | std::ios::out);

            binaryFile.write(file.type, 4);
            binaryFile.write((const char*) &file.version, sizeof(uint32_t));

            uint32_t jsonlength = file.json.size();
            binaryFile.write((const char*) &jsonlength, sizeof(uint32_t));

            uint32_t bloblength = file.binaryBlob.size();
            binaryFile.write((const char*) &bloblength, sizeof(uint32_t));

            binaryFile.write(file.json.data(), jsonlength);
            binaryFile.write(file.binaryBlob.data(), bloblength);

            binaryFile.close();

            return true;
        }

        bool loadAssetFile(const char* path, assetFile& file) {
            std::ifstream binaryFile;
            binaryFile.open(path, std::ios::binary | std::ios::in);

            if (binaryFile.is_open()) {
                binaryFile.read(file.type, 4);

                binaryFile.read((char*) &file.version, sizeof(uint32_t));

                uint32_t jsonlength;
                binaryFile.read((char*) &jsonlength, sizeof(uint32_t));
                uint32_t bloblength;
                binaryFile.read((char*) &bloblength, sizeof(uint32_t));

                file.json.resize(jsonlength);
                file.binaryBlob.resize(bloblength);

                binaryFile.read(file.json.data(), jsonlength);
                binaryFile.read(file.binaryBlob.data(), bloblength);

                binaryFile.close();

                return true;
            }
            else {
                return false;
            }
        }

        textureInfo readTextureInfo(assetFile* file) {
            textureInfo info;

            json textJson = json::parse(file->json);
            
            info.width = textJson["width"];
            info.height = textJson["height"];
            info.textureSize = textJson["textureSize"];
            info.format = textJson["format"];

            return info;
        }

        void unpackTexture(textureInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest) {
            LZ4_decompress_safe(sourcebuffer, dest, sourceSize, info->textureSize);
        }

        assetFile packTexture(textureInfo* info, void* pixelData) {
            assetFile file;
            file.type[0] = 'T';
            file.type[1] = 'E';
            file.type[2] = 'X';
            file.type[3] = 'T';
            file.version = 0;

            json textJson;
            textJson["width"] = info->width;
            textJson["height"] = info->height;
            textJson["textureSize"] = info->textureSize;
            textJson["format"] = textureFormat::RGBA8;
            file.json = textJson.dump();

            // compress buffer into blob
            int compressStaging = LZ4_compressBound(info->textureSize);
            file.binaryBlob.resize(compressStaging);
            int compressedSize = LZ4_compress_default((const char*) pixelData, file.binaryBlob.data(), info->textureSize, compressStaging);
            file.binaryBlob.resize(compressedSize);

            return file;
        }

        /* meshInfo readMeshInfo(assetFile* file) {
        
        }

        void unpackMesh(meshInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest) {

        }

        assetFile packMesh(meshInfo* info, void* pixelData) {

        } */
    }

}
