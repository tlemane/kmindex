#ifndef BLOCKDECOMPRESSORZSTD_H
#define BLOCKDECOMPRESSORZSTD_H

#include <BlockDecompressor.h>

#include <zstd.h>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressorZSTD : public BlockDecompressor
{
    private:
        //ZSTD options
        ZSTD_DCtx* context;        

        std::size_t decompress_buffer(std::size_t in_size) override;
    public:
        BlockDecompressorZSTD(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);
        BlockDecompressorZSTD(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);
        ~BlockDecompressorZSTD();
};

#endif