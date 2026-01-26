#ifndef BLOCKDECOMPRESSORLZ4_H
#define BLOCKDECOMPRESSORLZ4_H

#include <BlockDecompressor.h>

#include <lz4.h>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressorLZ4 : public BlockDecompressor
{
    private:
        std::size_t decompress_buffer(std::size_t in_size) override;

    public:
        BlockDecompressorLZ4(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);
        BlockDecompressorLZ4(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);
};

#endif