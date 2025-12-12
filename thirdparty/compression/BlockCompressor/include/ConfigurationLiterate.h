#ifndef CONFIGURATELITERATE_H
#define CONFIGURATELITERATE_H

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>

class ConfigurationLiterate
{
    public:
        ConfigurationLiterate(const std::string& filename = "", bool load_file = false);

        //We may want to write out configuration in already existing file
        //<read_file> should be set to false in that case to not overwriting current configuration
        void load(const std::string& filename, bool read_file = true);
        void read();
        void write() const;

        std::size_t get_nb_samples() const;
        std::size_t get_bit_vectors_per_block() const;
        std::uint8_t get_preset_level() const;

        void set_nb_samples(std::size_t nb_samples);
        void set_bit_vectors_per_block(std::size_t bit_vectors_per_block);
        void set_preset_level(std::uint8_t preset_level);
        
    private:
        //Default values are invalid for ensuring the use of a further proper configuration
        std::size_t nb_samples = 0;
        std::size_t bit_vectors_per_block = 0;
        std::uint8_t preset_level = 10;
        std::string filename;

        //Modify string <s> in lower case
        static std::string& to_lower_case(std::string& s);

        void assert_valid_config() const;

        void set_property(const std::string& property, std::size_t value);
};

#endif