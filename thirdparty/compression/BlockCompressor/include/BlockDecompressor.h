#ifndef BLOCKDECOMPRESSOR_H
#define BLOCKDECOMPRESSOR_H

#include <cassert>
#include <ConfigurationLiterate.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <sdsl/bit_vectors.hpp>
#include <sys/mman.h>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressor
{
    protected:
        //Properties
        ConfigurationLiterate config; //Configuration class { preset_level, bit_vectors_per_block, nb_samples }

        //Buffers and block variables
        std::vector<std::uint8_t> out_buffer; //Buffer to store block decoded data

        bool read_once = false; //Flag is set as soon as a block has been decoded
        std::size_t decoded_block_index = 0; //Store the last decoded block index
        std::size_t decoded_block_size = 0; //Store the last decoded block size
        std::size_t BLOCK_DECODED_SIZE = 0; //Expected size of a decoded block (in bytes)
        std::size_t bit_vector_size = 0; //Size of a decoded bit_vector (in bytes)
        //std::uint64_t minimum_hash;
        
        //IO variables
        int fd_matrix; //File descriptor of compressed matrix
        char* matrix = nullptr; //mmapped buffer
        char* in_buffer = nullptr; //Current position in matrix buffer
        std::size_t file_size = 0; //Compressed matrix size in bytes

        std::ifstream ef_in; //Input file stream of serialized Elias-Fano
        std::size_t header_size;

        //EF data
        sdsl::sd_vector<> ef; //Elias-Fano object used to store final blocks starting location
        sdsl::sd_vector<>::select_1_type ef_pos; //Select support for Elias-Fano
        std::uint64_t ef_size;

        //Decode i-th block if not currently loaded in memory
        void decode_block(std::size_t i);

        //Decompress "in_buffer" --> "out_buffer", must return the number of written bytes in out_buffer
        virtual std::size_t decompress_buffer(std::size_t in_size) = 0;
    public:
        BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, std::size_t header_size = 0);

        BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, std::size_t header_size = 0);

        virtual ~BlockDecompressor();

        //Decodes the block containing the corresponding hash value and that return the corresponding bit vector address
        //Returns nullptr if hash is out of range
        const std::uint8_t* get_bit_vector_from_hash(std::uint64_t hash);

        //Retrieve original matrix in a specified file
        void decompress_all(const std::string& out_path);

        //Mainly for testing purpose: 
        //Ask next query to decompress block even if it was already decompressed
        //This method doesn't free memory at all
        void unload();

        //Get size of bit vectors for loaded matrix
        inline std::uint64_t get_bit_vector_size() const
        {
            return bit_vector_size;
        }

};

#endif