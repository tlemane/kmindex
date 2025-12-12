#ifndef RNG_H
#define RNG_H

#include <random>
#include <stdexcept>

namespace bms
{
    class RNG
    {
        private:
            static std::mt19937 gen;
            static std::uint_fast32_t seed;
            RNG() = delete;
        public:
            static std::uint32_t rand_uint32_t(unsigned a, unsigned b);
            static std::int32_t rand_int32_t(int a, int b);
            static void set_seed(std::uint_fast32_t seed);
            static std::uint_fast32_t get_seed();
            static std::uint_fast32_t get_random_seed();
    };
};

#endif