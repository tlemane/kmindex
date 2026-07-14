#include <BlockCompressorLZ4.h>

//Return the number of written bytes
std::size_t BlockCompressorLZ4::compress_buffer(const std::uint8_t * const input, std::uint8_t * const output, std::size_t in_size, std::size_t out_size)
{
    return LZ4_compress_default(reinterpret_cast<const char*>(input), reinterpret_cast<char*>(output), in_size, out_size);
}

//Init LZ4 filters and resize output buffer according to estimated compressed block size
BlockCompressorLZ4::BlockCompressorLZ4(const std::string& output, const std::string& output_ef, const std::string& config_path) : BlockCompressor(output, output_ef, config_path)
{
    //Resize out buffer according to compressor upperbound
    resize_out_buffer(LZ4_compressBound(get_block_size()));
}