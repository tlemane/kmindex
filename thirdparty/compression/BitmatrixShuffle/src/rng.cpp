#include <rng.h>

namespace bms
{
    std::uint32_t RNG::rand_uint32_t(unsigned a, unsigned b) //Generate an unsigned integer in [a ; b[
    {
        if(a >= b)
            throw std::runtime_error("BMS-ERROR: RNG got unexpected interval");

        return gen() % (b-a) + a;
    }

    std::int32_t RNG::rand_int32_t(int a, int b) //Generate a signed integer in [a ; b[
    {
        if(a >= b)
            throw std::runtime_error("BMS-ERROR: RNG got unexpected interval");
        
        //Ensure generated number is positive when converted as a signed number
        return (std::int32_t)((gen() << 1) >> 1) % (b-a) + a;
    }

    std::uint_fast32_t RNG::get_seed()
    {
        return seed;
    }

    void RNG::set_seed(std::uint_fast32_t new_seed)
    {
        seed = new_seed;
        gen.seed(seed);
    }

    std::uint_fast32_t RNG::get_random_seed()
    {
        return std::random_device()();
    }

    std::uint_fast32_t RNG::seed = 42;
    std::mt19937 RNG::gen = std::mt19937(RNG::seed); // Standard mersenne_twister_engine seeded with default random_device
};