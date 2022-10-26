#ifndef QUERY_HPP_HIJEZMAU
#define QUERY_HPP_HIJEZMAU

#include <cstdint>
#include <atomic>
#include <string_view>
#include <cassert>

#include <kmtricks/hash.hpp>
#include <kmtricks/repartition.hpp>
#include <kmtricks/kmer.hpp>
#define WITH_XXHASH
#include <kmtricks/kmer_hash.hpp>

#include <kmindex/utils.hpp>
#include <kmindex/mer.hpp>

namespace kmq {

  class query_response
  {
    public:
      query_response(std::size_t n, std::size_t nbits, std::size_t width = 1);

      std::size_t block_size() const;

      std::uint8_t* get(std::size_t mer_pos);

    private:
      std::vector<std::uint8_t> m_responses;
      std::size_t m_block_size {0};
  };

  using query_response_t = std::unique_ptr<query_response>;

  class query
  {
    public:
      query(const std::string& name,
            const std::string& seq,
            std::size_t smer_size,
            std::size_t z_size,
            std::size_t nb_samples,
            double threshold,
            std::size_t width = 1);

      std::uint8_t* response_block(std::size_t mer_pos);

      std::size_t block_size() const;

      std::size_t size() const;

      std::size_t ksize() const;

      std::size_t zsize() const;

      std::string name() const;

      double threshold() const;

      smer_iterator iterate(std::size_t smer_size, smer_hasher* h);

    private:
      std::string m_name;
      std::string m_seq;
      std::size_t m_size;
      std::size_t m_ksize;
      std::size_t m_zsize;
      double m_threshold;
      query_response_t m_responses;
  };

}


#endif /* end of include guard: QUERY_HPP_HIJEZMAU */
