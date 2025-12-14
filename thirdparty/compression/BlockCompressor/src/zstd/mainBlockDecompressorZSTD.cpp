#include <BlockDecompressorZSTD.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char ** argv)
{
    if(argc != 6)
    {
        std::cout << "Usage: ./mainBlockDecompressorZSTD <config_file> <matrix> <ef_path> <header> <output>\n\n";
        return 1;
    }   

    std::string config_path = argv[1];
    std::string in_path = argv[2];
    std::string ef_path = argv[3];
    unsigned header_size = (unsigned)atoll(argv[4]);
    std::string output = argv[5];

    //Initialize decompressor, copies header and decompress each blocks to <output>
    BlockDecompressorZSTD(config_path, in_path, ef_path, header_size).decompress_all(output);
}