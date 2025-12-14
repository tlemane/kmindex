#include <BlockCompressor.h>

void BlockCompressor::append_block(const std::uint8_t * const input, std::size_t in_size)
{
    if(closed)
        return;

    if(input != in_buffer.data() && in_buffer_current_size != 0)
        throw std::runtime_error("Block was buffered and not flushed.");

    if(in_size % m_buffer.size() != 0)
        throw std::runtime_error("Block size is not a multiple of bit vector length");

    std::size_t out_size = compress_buffer(input, out_buffer.data(), in_size, out_buffer.size());

    //Add position to Elias-Fano encoder
    current_size += out_size;
    ef_pos.push_back(current_size);
    
    //Write bytes to output file
    m_out.write(reinterpret_cast<const char*>(out_buffer.data()), out_size * sizeof(std::uint8_t));
    m_out.flush();

    //Tell that internal buffer has been flushed
    in_buffer_current_size = 0;

    //If block isn't full, it means that it was the last block
    if(in_size != config.get_bit_vectors_per_block() * m_buffer.size())
        close();
}

BlockCompressor::BlockCompressor(const std::string& output, const std::string& output_ef, const std::string& config_path)
{
    ef_pos.push_back(0);

    config.load(config_path);

    //Configure buffer size according to parameters
    m_buffer.resize((config.get_nb_samples() + 7) / 8);

    m_out.open(output, std::ofstream::binary);
    ef_out.open(output_ef, std::ofstream::binary);

    in_buffer.resize(m_buffer.size() * config.get_bit_vectors_per_block());
}

BlockCompressor::~BlockCompressor()
{
    close();
}

void BlockCompressor::close()
{
    if(!closed)
    {
        //Write a smaller block if buffer isn't empty
        if(in_buffer_current_size != 0)
            append_block(in_buffer.data(), in_buffer_current_size);
        else
        {
            //Elias-Fano encoding
            write_elias_fano();

            //Close file descriptors
            ef_out.close();
            m_out.close();
        }
    }

    closed = true;
}

void BlockCompressor::write_elias_fano()
{
    std::uint64_t ef_size = ef_pos.size();

    //ef_out.write(reinterpret_cast<const char*>(&minimum_hash), sizeof(std::uint64_t));
    ef_out.write(reinterpret_cast<const char*>(&ef_size), sizeof(std::uint64_t));
    
    //Create Elias-Fano representation from positions
    sdsl::sd_vector<> ef(ef_pos.begin(), ef_pos.end());
    sdsl::serialize(ef, ef_out);
}

void BlockCompressor::append_zero_buffers(std::uint64_t n)
{
    if(closed)
        return;

    if(n >= 1)
    {
        std::fill(m_buffer.begin(), m_buffer.end(), 0);
        for(std::uint64_t i = 0; i < n; ++i)
            append_bit_vector(m_buffer.data());
    }
}

void BlockCompressor::append_bit_vector(const std::uint8_t * const bit_vector)
{
    if(closed)
        return;

    //Append bit vector to block
    std::memcpy(in_buffer.data()+in_buffer_current_size, bit_vector, m_buffer.size());
    
    ++bit_vectors_read;

    // Update variables tracking data to use in in_buffer vector
    in_buffer_current_size = m_buffer.size() * (bit_vectors_read % config.get_bit_vectors_per_block());

    if(bit_vectors_read % config.get_bit_vectors_per_block() == 0)
    {
        int block_size = in_buffer_current_size ? in_buffer_current_size : in_buffer.size();

        // Compress block
        append_block(in_buffer.data(), block_size);
    }
}

void BlockCompressor::compress_file(const std::string& in_path, std::size_t header_size)
{
    if(closed)
        return;

    if(!std::filesystem::exists(in_path))
        throw std::runtime_error("Partition file '" + in_path + "' was not found.");

    std::ifstream in_file(in_path, std::ifstream::binary);
    
    in_file.seekg(0, std::ifstream::end);

    if(in_file.tellg() == -1)
        throw std::runtime_error("Unexpected file position");

        
    if(header_size > (std::size_t)in_file.tellg())
        throw std::runtime_error("header size is greater than file size");

    std::uint64_t size = (std::size_t)in_file.tellg() - header_size;

    write_header(in_file, header_size);

    const std::uint64_t ROW_LENGTH = m_buffer.size();
    const std::uint64_t BLOCK_SIZE = in_buffer.size();

    if(size % ROW_LENGTH != 0)
        throw std::runtime_error("File size doesn't match [<bit_vector>], check the header size or file size");

    //Write full blocks
    const std::uint64_t NB_FULL_BLOCKS = size / BLOCK_SIZE;

    for(std::uint64_t i = 0; i < NB_FULL_BLOCKS; ++i)
    {
        in_file.read(reinterpret_cast<char*>(in_buffer.data()), BLOCK_SIZE);
        append_block(in_buffer.data(), BLOCK_SIZE);
    }

    //Write last block
    if(size % BLOCK_SIZE != 0)
    {
        //Preset buffer tracking variable
        in_buffer_current_size = size % BLOCK_SIZE; //Remaining bit_vectors

        in_file.read(reinterpret_cast<char*>(in_buffer.data()), in_buffer_current_size);
        append_block(in_buffer.data(), in_buffer_current_size);
    }

    close();
}

void BlockCompressor::write_header(std::ifstream& in_file, std::size_t header_size)
{
    in_file.seekg(0, std::ifstream::beg);

    char * header = new char[header_size];

    //Read header
    in_file.read(header, header_size);

    //Write header
    m_out.write(header, header_size);

    delete[] header;
}

void BlockCompressor::write_header(const char * const header, std::size_t header_size)
{
    m_out.write(header, header_size);
}

bool BlockCompressor::is_closed() const
{
    return closed;
}

std::size_t BlockCompressor::get_block_size() const
{
    return in_buffer.size();
}

void BlockCompressor::resize_out_buffer(std::size_t size)
{
    out_buffer.resize(size);
}