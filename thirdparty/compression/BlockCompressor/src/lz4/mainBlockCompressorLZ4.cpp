#include <BlockCompressorLZ4.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char ** argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: ./mainBlockCompressorLZ4 <config> <matrix> <header> <output>\n\n";
        return 1;
    }

    std::string config_path = argv[1];
    std::string in_path = argv[2];
    unsigned header_size = (unsigned)std::atoll(argv[3]);
    std::string output = argv[4];

    BlockCompressorLZ4(output, output + ".ef", config_path).compress_file(in_path, header_size);
}