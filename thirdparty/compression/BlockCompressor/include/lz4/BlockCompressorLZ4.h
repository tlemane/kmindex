#ifndef BLOCKCOMPRESSORLZ4_H
#define BLOCKCOMPRESSORLZ4_H

#include <BlockCompressor.h>

//LZ4 utils main header
#include <lz4.h>

class BlockCompressorLZ4 : public BlockCompressor
{
    private:
        //Write out current block
        std::size_t compress_buffer(const std::uint8_t * input, std::uint8_t * output, std::size_t in_size, std::size_t out_size) override;
        
    public:
        BlockCompressorLZ4(const std::string& output, const std::string& output_ef, const std::string& config_path);
};

#endif