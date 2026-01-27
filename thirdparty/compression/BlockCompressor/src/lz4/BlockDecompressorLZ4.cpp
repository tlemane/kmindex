#include <BlockDecompressorLZ4.h>

BlockDecompressorLZ4::BlockDecompressorLZ4(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size) : BlockDecompressorLZ4(ConfigurationLiterate(config_path, true), matrix_path, ef_path, header_size) {}

BlockDecompressorLZ4::BlockDecompressorLZ4(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size) : BlockDecompressor(config, matrix_path, ef_path, header_size)
{
    //Init options
    //No options are needed to be initialized when decompressing with LZ4
}


std::size_t BlockDecompressorLZ4::decompress_buffer(std::size_t in_size)
{
    return LZ4_decompress_safe(in_buffer, reinterpret_cast<char*>(out_buffer.data()), in_size, out_buffer.size());
}