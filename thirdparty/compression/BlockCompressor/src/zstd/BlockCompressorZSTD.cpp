#include <BlockCompressorZSTD.h>

//Return the number of written bytes
std::size_t BlockCompressorZSTD::compress_buffer(const std::uint8_t * const input, std::uint8_t * const output, std::size_t in_size, std::size_t out_size)
{
    return ZSTD_compress2(context, output, out_size, input, in_size);
}

//Init ZSTD filters and resize output buffer according to estimated compressed block size
BlockCompressorZSTD::BlockCompressorZSTD(const std::string& output, const std::string& output_ef, const std::string& config_path) : BlockCompressor(output, output_ef, config_path)
{
    //Configure options and filters (compression level) 
    context = ZSTD_createCCtx();

    ZSTD_CCtx_setParameter(context, ZSTD_c_compressionLevel, config.get_preset_level());

    //Resize out buffer according to compressor upperbound
    resize_out_buffer(ZSTD_compressBound(get_block_size()));
}

BlockCompressorZSTD::~BlockCompressorZSTD()
{
    //Free memory
    ZSTD_freeCCtx(context);
}
