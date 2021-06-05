#include "vk_engine/assets/assets.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace vk_engine {

    namespace assests {

        bool saveBinaryFile(const char* path, const assestFile& file) {
            std::ostream binaryFile;
            binaryFile.open(path, std::ios::binary | std::ios::out);

            binaryFile.write(file.type, 4);
            binaryFile.write((const char*) &version, sizeof(uint32_t));

            uint32_t jsonlength = file.json.size();
            binaryFile.write((const char*) &jsonlength, sizeof(uint32_t));

            uint32_t bloblength = file.binaryBlob.size();
            binaryFile.write((const char*) &bloblength, sizeof(uint32_t));

            binaryFile.write(file.json.data(), jsonlength);
            binaryFile.write(file.binaryBlob.data(), bloblength);

            binaryFile.close();

            return true;
        };

        bool loadBinaryFile(const char* path, assestFile& file) {
            std::ifstream binaryFile;
            binaryFile.open(path, std::ios::binary | std::ios::in);

            if (binaryFile.is_open()) {
                binaryFile.read(file.type, 4);
                binaryFile.read((char*) file.version, sizeof(uint32_t));

                uint32_t jsonlength;
                binaryFile.read((char*) jsonlength, sizeof(uint32_t));

                uint32_t bloblength;
                binaryFile.read((char*) bloblength, sizeof(uint32_t));

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
        };

        textureInfo readTextureInfo(assestFile* file) {
            textureInfo info;
            
            info.textureSize = file.binaryBlob.size();
            info.pixelSize[0] = width;
            info.pixelSize[1] = height;
            info.pixelSize[2] = bufferSize;
            info.pixels(file.binaryBlob.begin(), file.binaryBlob.end());
        };

        void unpackTexture(textureInfo* info, const char* sourcebuffer, size_t sourceSize, char* dest) {

        };

        assestFile packTexture(textureInfo* info, void* pixelData) {

        };

    }

}
