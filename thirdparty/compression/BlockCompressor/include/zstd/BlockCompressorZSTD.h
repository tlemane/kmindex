#ifndef BLOCKCOMPRESSORZSTD_H
#define BLOCKCOMPRESSORZSTD_H

#include <BlockCompressor.h>

//Zstd utils main header
#include <zstd.h>

class BlockCompressorZSTD : public BlockCompressor
{
    private:
        //ZSTD options
        ZSTD_CCtx* context;

        //Write out current block
        std::size_t compress_buffer(const std::uint8_t * input, std::uint8_t * output, std::size_t in_size, std::size_t out_size) override;
        
    public:
        BlockCompressorZSTD(const std::string& output, const std::string& output_ef, const std::string& config_path);
        ~BlockCompressorZSTD();
};

#endif