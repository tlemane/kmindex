#ifndef BLOCKCOMPRESSOR_H
#define BLOCKCOMPRESSOR_H

#include <cassert>

//Configuration parser
#include <ConfigurationLiterate.h>

#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <filesystem>

//Elias-Fano encoding
#include <sdsl/bit_vectors.hpp>


class BlockCompressor
{
    protected:
        void resize_out_buffer(std::size_t size);

        //Properties
        ConfigurationLiterate config; //Configuration class { preset_level, bit_vectors_per_block, nb_samples }
        
    private:
    
        bool closed = false;
        
        std::vector<std::uint8_t> m_buffer;
        std::vector<std::uint8_t> in_buffer;
        std::vector<std::uint8_t> out_buffer;
        
        //Buffers and IO variables
        std::uint64_t current_size = 0;

        std::size_t in_buffer_current_size = 0;
        std::size_t bit_vectors_read = 0;
        std::ofstream m_out;
        
        //EF data
        std::ofstream ef_out;
        std::vector<std::uint64_t> ef_pos;

        //Compress buffer
        virtual std::size_t compress_buffer(const std::uint8_t * input, std::uint8_t * output, std::size_t in_size, std::size_t out_size) = 0;

        //Write out Elias-Fano representation of blocks starting position
        void write_elias_fano();
        
        
        public:
        BlockCompressor(const std::string& output, const std::string& output_ef, const std::string& config_path);
        virtual ~BlockCompressor();
        
        //Writes header from input file
        void write_header(std::ifstream& in_file, std::size_t header_size);

        //Writes header from user input
        void write_header(const char * const header, std::size_t header_size);

        //Append bit vector to block buffer
        void append_bit_vector(const std::uint8_t * const bit_vector);

        //Write out current block
        void append_block(const std::uint8_t * const input, std::size_t in_size);

        //Write out <n> bit_vectors filled with zeroes
        void append_zero_buffers(std::uint64_t n);

        //Close file descriptors, flush last block
        void close();

        //Compress hash:bf:bin kmtricks matrix
        void compress_file(const std::string& in_path, std::size_t header_size = 0);
        
        std::size_t get_block_size() const;

        bool is_closed() const;
};

#endif