#include <BlockDecompressor.h>

BlockDecompressor::BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, std::size_t header_size) : BlockDecompressor(ConfigurationLiterate(config_path, true), matrix_path, ef_path, header_size) {}

BlockDecompressor::BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, std::size_t header_size)
{
    this->config = ConfigurationLiterate(config);
    this->header_size = header_size;

    //Open file descriptor to mmap file
    this->fd_matrix = open(matrix_path.c_str(), O_RDONLY);
    this->file_size = lseek(fd_matrix, 0, SEEK_END);
    this->matrix = (char*)mmap(nullptr, this->file_size, PROT_READ, MAP_PRIVATE, fd_matrix, 0);

    //Tell system that data will be accessed sequentially
    posix_madvise(this->matrix, this->file_size, POSIX_MADV_SEQUENTIAL);

    //Initialize variables according to configurations
    bit_vector_size = ((config.get_nb_samples() + 7) / 8);
    BLOCK_DECODED_SIZE = bit_vector_size * config.get_bit_vectors_per_block();

    out_buffer.resize(BLOCK_DECODED_SIZE);
    
    //Deserialize size + EF
    ef_in.open(ef_path, std::ifstream::binary);
    ef_in.read(reinterpret_cast<char*>(&ef_size), sizeof(std::uint64_t)); //Retrieve size
    sdsl::load(ef, ef_in);
    sdsl::util::init_support(ef_pos, &ef);
}


void BlockDecompressor::decode_block(std::size_t i)
{
    //Retrieve from EF encoding the location of corresponding block and its size
    std::size_t pos_a = ef_pos(i+1);
    std::size_t pos_b = ef_pos(i+2); 

    std::size_t block_encoded_size = pos_b - pos_a;

    //Set cursor to block starting location
    in_buffer = matrix + pos_a + header_size;

    decoded_block_size = decompress_buffer(block_encoded_size);
    
    //Check if decoded size match expected size (taking into account the possibility of a smaller last block)
    if(decoded_block_size != BLOCK_DECODED_SIZE && (i + 2 != ef_size))
        throw std::runtime_error("Decoded block got an unexpected size: " + std::to_string(decoded_block_size) + " (should have been " + std::to_string(BLOCK_DECODED_SIZE) + ")");

    decoded_block_index = i;
}

//Hash range must be shifted from 0 to maximum_hash-minimum_hash
const std::uint8_t* BlockDecompressor::get_bit_vector_from_hash(std::uint64_t hash)
{
    //Get block index
    std::uint64_t block_index = hash / config.get_bit_vectors_per_block();
    //Get index in block
    std::uint64_t hash_index = hash % config.get_bit_vectors_per_block();

    //Handle queried hashes that are out of matrix
    if(block_index + 1 >= ef_size)
        return nullptr;

    //Handle last block out index
    if(read_once && hash_index >= decoded_block_size / get_bit_vector_size())
        return nullptr;

    //Avoid decompressing a block that was decompressed on last call
    if(block_index != decoded_block_index || !read_once) 
    {
        read_once = true;
        decode_block(block_index);
    }

    //Return corresponding bit_vector
    return out_buffer.data() + hash_index * get_bit_vector_size();
}

void BlockDecompressor::decompress_all(const std::string& out_path)
{
    std::ofstream out_file;
    out_file.open(out_path, std::ofstream::binary);

    //Write out header
    out_file.write(matrix, header_size);
    
    //i < ef_size - 1 but as it is unsigned, avoid possible underflows if ef_pos is empty (but it should never occur)
    for(std::size_t i = 0; i + 1 < ef_size; ++i)
    {
        decode_block(i);
        out_file.write(reinterpret_cast<const char*>(out_buffer.data()), decoded_block_size);
    }
}

void BlockDecompressor::unload()
{
    read_once = false;
}
