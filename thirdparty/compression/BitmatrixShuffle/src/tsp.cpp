#include <tsp.h>
#include <deque>

//AVX2/SSE2
#define SIMDE_ENABLE_NATIVE_ALIASES
#include <x86/sse2.h>
#include <x86/avx.h>
#include <x86/avx2.h>
#include <stdint.h>

const uint8_t lookup8bit[256] = {
    /* 0 */ 0, /* 1 */ 1, /* 2 */ 1, /* 3 */ 2,
    /* 4 */ 1, /* 5 */ 2, /* 6 */ 2, /* 7 */ 3,
    /* 8 */ 1, /* 9 */ 2, /* a */ 2, /* b */ 3,
    /* c */ 2, /* d */ 3, /* e */ 3, /* f */ 4,
    /* 10 */ 1, /* 11 */ 2, /* 12 */ 2, /* 13 */ 3,
    /* 14 */ 2, /* 15 */ 3, /* 16 */ 3, /* 17 */ 4,
    /* 18 */ 2, /* 19 */ 3, /* 1a */ 3, /* 1b */ 4,
    /* 1c */ 3, /* 1d */ 4, /* 1e */ 4, /* 1f */ 5,
    /* 20 */ 1, /* 21 */ 2, /* 22 */ 2, /* 23 */ 3,
    /* 24 */ 2, /* 25 */ 3, /* 26 */ 3, /* 27 */ 4,
    /* 28 */ 2, /* 29 */ 3, /* 2a */ 3, /* 2b */ 4,
    /* 2c */ 3, /* 2d */ 4, /* 2e */ 4, /* 2f */ 5,
    /* 30 */ 2, /* 31 */ 3, /* 32 */ 3, /* 33 */ 4,
    /* 34 */ 3, /* 35 */ 4, /* 36 */ 4, /* 37 */ 5,
    /* 38 */ 3, /* 39 */ 4, /* 3a */ 4, /* 3b */ 5,
    /* 3c */ 4, /* 3d */ 5, /* 3e */ 5, /* 3f */ 6,
    /* 40 */ 1, /* 41 */ 2, /* 42 */ 2, /* 43 */ 3,
    /* 44 */ 2, /* 45 */ 3, /* 46 */ 3, /* 47 */ 4,
    /* 48 */ 2, /* 49 */ 3, /* 4a */ 3, /* 4b */ 4,
    /* 4c */ 3, /* 4d */ 4, /* 4e */ 4, /* 4f */ 5,
    /* 50 */ 2, /* 51 */ 3, /* 52 */ 3, /* 53 */ 4,
    /* 54 */ 3, /* 55 */ 4, /* 56 */ 4, /* 57 */ 5,
    /* 58 */ 3, /* 59 */ 4, /* 5a */ 4, /* 5b */ 5,
    /* 5c */ 4, /* 5d */ 5, /* 5e */ 5, /* 5f */ 6,
    /* 60 */ 2, /* 61 */ 3, /* 62 */ 3, /* 63 */ 4,
    /* 64 */ 3, /* 65 */ 4, /* 66 */ 4, /* 67 */ 5,
    /* 68 */ 3, /* 69 */ 4, /* 6a */ 4, /* 6b */ 5,
    /* 6c */ 4, /* 6d */ 5, /* 6e */ 5, /* 6f */ 6,
    /* 70 */ 3, /* 71 */ 4, /* 72 */ 4, /* 73 */ 5,
    /* 74 */ 4, /* 75 */ 5, /* 76 */ 5, /* 77 */ 6,
    /* 78 */ 4, /* 79 */ 5, /* 7a */ 5, /* 7b */ 6,
    /* 7c */ 5, /* 7d */ 6, /* 7e */ 6, /* 7f */ 7,
    /* 80 */ 1, /* 81 */ 2, /* 82 */ 2, /* 83 */ 3,
    /* 84 */ 2, /* 85 */ 3, /* 86 */ 3, /* 87 */ 4,
    /* 88 */ 2, /* 89 */ 3, /* 8a */ 3, /* 8b */ 4,
    /* 8c */ 3, /* 8d */ 4, /* 8e */ 4, /* 8f */ 5,
    /* 90 */ 2, /* 91 */ 3, /* 92 */ 3, /* 93 */ 4,
    /* 94 */ 3, /* 95 */ 4, /* 96 */ 4, /* 97 */ 5,
    /* 98 */ 3, /* 99 */ 4, /* 9a */ 4, /* 9b */ 5,
    /* 9c */ 4, /* 9d */ 5, /* 9e */ 5, /* 9f */ 6,
    /* a0 */ 2, /* a1 */ 3, /* a2 */ 3, /* a3 */ 4,
    /* a4 */ 3, /* a5 */ 4, /* a6 */ 4, /* a7 */ 5,
    /* a8 */ 3, /* a9 */ 4, /* aa */ 4, /* ab */ 5,
    /* ac */ 4, /* ad */ 5, /* ae */ 5, /* af */ 6,
    /* b0 */ 3, /* b1 */ 4, /* b2 */ 4, /* b3 */ 5,
    /* b4 */ 4, /* b5 */ 5, /* b6 */ 5, /* b7 */ 6,
    /* b8 */ 4, /* b9 */ 5, /* ba */ 5, /* bb */ 6,
    /* bc */ 5, /* bd */ 6, /* be */ 6, /* bf */ 7,
    /* c0 */ 2, /* c1 */ 3, /* c2 */ 3, /* c3 */ 4,
    /* c4 */ 3, /* c5 */ 4, /* c6 */ 4, /* c7 */ 5,
    /* c8 */ 3, /* c9 */ 4, /* ca */ 4, /* cb */ 5,
    /* cc */ 4, /* cd */ 5, /* ce */ 5, /* cf */ 6,
    /* d0 */ 3, /* d1 */ 4, /* d2 */ 4, /* d3 */ 5,
    /* d4 */ 4, /* d5 */ 5, /* d6 */ 5, /* d7 */ 6,
    /* d8 */ 4, /* d9 */ 5, /* da */ 5, /* db */ 6,
    /* dc */ 5, /* dd */ 6, /* de */ 6, /* df */ 7,
    /* e0 */ 3, /* e1 */ 4, /* e2 */ 4, /* e3 */ 5,
    /* e4 */ 4, /* e5 */ 5, /* e6 */ 5, /* e7 */ 6,
    /* e8 */ 4, /* e9 */ 5, /* ea */ 5, /* eb */ 6,
    /* ec */ 5, /* ed */ 6, /* ee */ 6, /* ef */ 7,
    /* f0 */ 4, /* f1 */ 5, /* f2 */ 5, /* f3 */ 6,
    /* f4 */ 5, /* f5 */ 6, /* f6 */ 6, /* f7 */ 7,
    /* f8 */ 5, /* f9 */ 6, /* fa */ 6, /* fb */ 7,
    /* fc */ 6, /* fd */ 7, /* fe */ 7, /* ff */ 8
};

 namespace AVX2_harley_seal
 {
    __m256i popcount(const __m256i v)
    {
        const __m256i m1 = _mm256_set1_epi8(0x55);
        const __m256i m2 = _mm256_set1_epi8(0x33);
        const __m256i m4 = _mm256_set1_epi8(0x0F);

        const __m256i t1 = _mm256_sub_epi8(v,       (_mm256_srli_epi16(v,  1) & m1));
        const __m256i t2 = _mm256_add_epi8(t1 & m2, (_mm256_srli_epi16(t1, 2) & m2));
        const __m256i t3 = _mm256_add_epi8(t2, _mm256_srli_epi16(t2, 4)) & m4;
        return _mm256_sad_epu8(t3, _mm256_setzero_si256());
    }

    void CSA(__m256i& h, __m256i& l, __m256i a, __m256i b, __m256i c)
    {
        const __m256i u = a ^ b;
        h = (a & b) | (u & c);
        l = u ^ c;
    }


    #define XOR_UNALIGNED(x,y) (_mm256_loadu_si256((x)) ^ _mm256_loadu_si256((y)))
    uint64_t hamming_distance_unaligned(const __m256i* a, const __m256i* b, const uint64_t size)
    {
        __m256i total     = _mm256_setzero_si256();
        __m256i ones      = _mm256_setzero_si256();
        __m256i twos      = _mm256_setzero_si256();
        __m256i fours     = _mm256_setzero_si256();
        __m256i eights    = _mm256_setzero_si256();
        __m256i sixteens  = _mm256_setzero_si256();
        __m256i twosA, twosB, foursA, foursB, eightsA, eightsB;

        const std::uint64_t limit = size - size % 16;
        std::uint64_t i = 0;

        for(; i < limit; i += 16)
        {
            CSA(twosA, ones, ones, XOR_UNALIGNED(a+i+0, b+i+0), XOR_UNALIGNED(a+i+1, b+i+1));
            CSA(twosB, ones, ones, XOR_UNALIGNED(a+i+2, b+i+2), XOR_UNALIGNED(a+i+3, b+i+3));
            CSA(foursA, twos, twos, twosA, twosB);
            CSA(twosA, ones, ones, XOR_UNALIGNED(a+i+4, b+i+4), XOR_UNALIGNED(a+i+5, b+i+5));
            CSA(twosB, ones, ones, XOR_UNALIGNED(a+i+6, b+i+6), XOR_UNALIGNED(a+i+7, b+i+7));
            CSA(foursB, twos, twos, twosA, twosB);
            CSA(eightsA,fours, fours, foursA, foursB);
            CSA(twosA, ones, ones, XOR_UNALIGNED(a+i+8, b+i+8), XOR_UNALIGNED(a+i+9, b+i+9));
            CSA(twosB, ones, ones, XOR_UNALIGNED(a+i+10, b+i+10), XOR_UNALIGNED(a+i+11, b+i+11));
            CSA(foursA, twos, twos, twosA, twosB);
            CSA(twosA, ones, ones, XOR_UNALIGNED(a+i+12, b+i+12), XOR_UNALIGNED(a+i+13, b+i+13));
            CSA(twosB, ones, ones, XOR_UNALIGNED(a+i+14, b+i+14), XOR_UNALIGNED(a+i+15, b+i+15));
            CSA(foursB, twos, twos, twosA, twosB);
            CSA(eightsB, fours, fours, foursA, foursB);
            CSA(sixteens, eights, eights, eightsA, eightsB);

            total = _mm256_add_epi64(total, popcount(sixteens));
        }

        total = _mm256_slli_epi64(total, 4);     // * 16
        total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount(eights), 3)); // += 8 * ...
        total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount(fours),  2)); // += 4 * ...
        total = _mm256_add_epi64(total, _mm256_slli_epi64(popcount(twos),   1)); // += 2 * ...
        total = _mm256_add_epi64(total, popcount(ones));

        for(; i < size; i++)
            total = _mm256_add_epi64(total, popcount(XOR_UNALIGNED(a+i, b+i)));


        return static_cast<std::uint64_t>(_mm256_extract_epi64(total, 0))
             + static_cast<std::uint64_t>(_mm256_extract_epi64(total, 1))
             + static_cast<std::uint64_t>(_mm256_extract_epi64(total, 2))
             + static_cast<std::uint64_t>(_mm256_extract_epi64(total, 3));
    }
    #undef XOR_UNALIGNED

} // AVX2_harley_seal

namespace bms {

    //TSP path, returns the number of computed distances
    std::size_t build_NN(const char* const MATRIX, DistanceMatrix& distanceMatrix, const std::size_t SUBSAMPLED_ROWS, const std::size_t OFFSET, std::vector<std::uint64_t>& order)
    {
        //Pick a random first vertex
        std::uint64_t firstVertex = RNG::rand_uint32_t(0, distanceMatrix.width());

        //Vector of added vertices (set true for the first vertex)
        std::vector<bool> alreadyAdded;
        alreadyAdded.resize(distanceMatrix.width());
        alreadyAdded[firstVertex] = true;

        //Deque for building path with first vertex as starting point
        std::vector<std::uint64_t> path = {firstVertex};
        path.reserve(distanceMatrix.width());

        //Build vector of indices for VPTree
        std::vector<std::uint64_t> vertices;
        vertices.resize(distanceMatrix.width());
        for(std::size_t i = 0; i < vertices.size(); ++i)
            vertices[i] = i;

        std::size_t counter = 0;

        DistanceFunctions df = VPTree<std::uint64_t>::bind_distance_functions(
            [=, &counter](std::uint64_t a, std::uint64_t b) -> double {
                ++counter;
                return columns_hamming_distance(MATRIX, SUBSAMPLED_ROWS, a+OFFSET, b+OFFSET);
            },
            [&distanceMatrix](std::uint64_t a, std::uint64_t b) -> double { return distanceMatrix.get(a, b); },
            [&distanceMatrix](std::uint64_t a, std::uint64_t b, double d) { distanceMatrix.set(a, b, d); }
        );

        VPTree<std::uint64_t> root(vertices, &df);

        //Added second vertex to data structures
        //Find next vertices to add by checking which is the minimum to take
        for(std::size_t i = 1; i < distanceMatrix.width(); ++i)
        {
            IndexDistance match = find_closest_vertex(root, path[i-1], alreadyAdded);
            alreadyAdded[match.index] = true;
            path.push_back(match.index);
        }

        //std::cout << "\tComputed distances (VPTree): " << counter << "/" << (distanceMatrix.width() * (distanceMatrix.width() - 1) / 2) <<  std::endl;

        //Store global order
        for(std::size_t i = 0; i < distanceMatrix.width(); ++i)
            order[i+OFFSET] = path[i] + OFFSET; //Add offset because columns are addressed by their global location

        return counter;
    }


    //TSP path filled by both ends, less sensitive of the first chosen vertex, returns the number of computed distances
    std::size_t build_double_ended_NN(const char* const MATRIX, DistanceMatrix& distanceMatrix, const std::size_t SUBSAMPLED_ROWS, const std::size_t OFFSET, std::vector<std::uint64_t>& order)
    {
        //Pick a random first vertex
        std::uint64_t firstVertex = RNG::rand_uint32_t(0, distanceMatrix.width());

        //Vector of added vertices (set true for the first vertex)
        std::vector<bool> alreadyAdded;
        alreadyAdded.resize(distanceMatrix.width());
        alreadyAdded[firstVertex] = true;

        //Deque for building path with first vertex as starting point
        std::deque<std::uint64_t> orderDeque = {firstVertex};

        //Build vector of indices for VPTree
        std::vector<std::uint64_t> vertices;
        vertices.resize(distanceMatrix.width());
        for(std::size_t i = 0; i < vertices.size(); ++i)
            vertices[i] = i;

        std::size_t counter = 0;

        //Use counter to count how many distance computation could be avoided by using a VPTree
        DistanceFunctions df = VPTree<std::uint64_t>::bind_distance_functions(
            [=, &counter](std::uint64_t a, std::uint64_t b) -> double {
                ++counter;
                return columns_hamming_distance(MATRIX, SUBSAMPLED_ROWS, a+OFFSET, b+OFFSET);
            },
            [&distanceMatrix](std::uint64_t a, std::uint64_t b) -> double { return distanceMatrix.get(a, b); },
            [&distanceMatrix](std::uint64_t a, std::uint64_t b, double d) { distanceMatrix.set(a, b, d); }
        );

        VPTree<std::uint64_t> root(vertices, &df);

        //Find second vertex
        IndexDistance second = find_closest_vertex(root, firstVertex, alreadyAdded);

        //Added second vertex to data structures
        orderDeque.push_back(second.index);
        alreadyAdded[second.index] = true;

        //Find closest vertices from path front and back
        IndexDistance a = find_closest_vertex(root, orderDeque.front(), alreadyAdded);
        IndexDistance b = find_closest_vertex(root, orderDeque.back(), alreadyAdded);

        //Find next vertices to add by checking which is the minimum to take
        for(std::size_t i = 2; i < distanceMatrix.width(); ++i)
        {
            if(a.distance < b.distance)
            {
                orderDeque.push_front(a.index);
                alreadyAdded[a.index] = true;

                if(a.index == b.index)
                    b = find_closest_vertex(root, orderDeque.back(), alreadyAdded);

                a = find_closest_vertex(root, orderDeque.front(), alreadyAdded);
            }
            else
            {
                orderDeque.push_back(b.index);
                alreadyAdded[b.index] = true;

                if(b.index == a.index)
                    a = find_closest_vertex(root, orderDeque.front(), alreadyAdded);

                b = find_closest_vertex(root, orderDeque.back(), alreadyAdded);
            }
        }

        //std::cout << "\tComputed distances (VPTree): " << counter << "/" << (distanceMatrix.width() * (distanceMatrix.width() - 1) / 2) <<  std::endl;

        //Store global order
        for(std::size_t i = 0; i < distanceMatrix.width(); ++i)
            order[i+OFFSET] = orderDeque[i] + OFFSET; //Add offset because columns are addressed by their global location

        return counter;
    }

    IndexDistance find_closest_vertex(VPTree<std::uint64_t>& VPTREE, const std::uint64_t VERTEX, const std::vector<bool>& ALREADY_ADDED)
    {
        IndexDistance nn = {0, 2.0};
        VPTREE.get_unvisited_nearest_neighbor(VERTEX, ALREADY_ADDED, &nn.distance, &nn.index);

        return nn;
    }

    std::size_t hamming_distance(const std::uint8_t* a, const std::uint8_t* b, const std::size_t size)
    {
        std::size_t total = AVX2_harley_seal::hamming_distance_unaligned((const __m256i*)a, (const __m256i*)b, size / 32);

        for (size_t i = size - size % 32; i < size; i++)
            total += lookup8bit[a[i] ^ b[i]];

        return total;
    }

    double columns_hamming_distance(const char* const transposed_matrix, const std::size_t MAX_ROW, const std::uint64_t COLUMN_A, const std::uint64_t COLUMN_B)
    {
        return 1.0*hamming_distance(reinterpret_cast<const std::uint8_t*>(transposed_matrix + (COLUMN_A)*(MAX_ROW/8)), reinterpret_cast<const std::uint8_t*>(transposed_matrix + (COLUMN_B)*(MAX_ROW/8)), MAX_ROW/8) / MAX_ROW;
    }
};
