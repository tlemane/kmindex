#ifndef QUERY_HPP_HIJEZMAU
#define QUERY_HPP_HIJEZMAU

#include <cstdint>
#include <atomic>
#include <string_view>
#include <cassert>

#ifndef KMTRICKS_PUBLIC
  #define KMTRICKS_PUBLIC
#endif

#include <array>
#include <kmindex/index/common.hpp>
#include <kmindex/utils.hpp>
#include <kmindex/spinlock.hpp>

namespace kmq {

  class query_response
  {
    public:
      query_response(const std::string& name, std::size_t n, std::size_t nbits, std::size_t width = 1);

      std::size_t block_size() const;
      std::size_t nbk() const;
      const std::string& name() const;

      std::uint8_t* get(std::size_t mer_pos);
      void free();

    private:
      std::string m_name;
      std::vector<std::uint8_t> m_responses;
      std::size_t m_block_size {0};
  };

  using query_response_t = std::unique_ptr<query_response>;

  class batch_query
  {
    using qsmer_type = std::pair<smer, std::uint32_t>;
    using qpart_type = std::vector<qsmer_type>;
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;

    public:
      batch_query(std::size_t nb_samples,
                  std::size_t nb_partitions,
                  std::size_t smer_size,
                  std::size_t z_size,
                  std::size_t width,
                  repart_type repart,
                  hw_type hw,
                  std::size_t minim_size)
        : m_nb_samples(nb_samples),
          m_nb_parts(nb_partitions),
          m_smer_size(smer_size),
          m_z_size(z_size),
          m_width(width),
          m_repart(repart),
          m_hw(hw),
          m_msize(minim_size),
          m_smers(m_nb_parts)
      {
      }

    public:
      void reserve(std::size_t nb_queries)
      {
        m_responses.reserve(nb_queries);
      }

      void add_query(const std::string name,
                     const std::string& seq)
      {
        std::size_t n = seq.size() - m_smer_size + 1;

        m_responses.push_back(
            std::make_unique<query_response>(std::move(name), n, m_nb_samples, m_width));

        std::uint32_t qid = m_responses.size() - 1;

        loop_executor<MAX_KMER_SIZE>::exec<smer_functor>(m_smer_size, m_smers, seq, qid, m_smer_size, m_repart, m_hw, m_msize);
      }

      qpart_type& partition(std::size_t p)
      {
        return m_smers[p];
      }

      std::vector<query_response_t>& response()
      {
        return m_responses;
      }

      auto begin()
      {
        return m_smers.begin();
      }

      auto end()
      {
        return m_smers.end();
      }

      void free_smers()
      {
        free_container(m_smers);
      }

      void free_responses()
      {
        free_container(m_responses);
      }

      std::size_t size() const
      {
        return m_responses.size();
      }

    private:

    private:
      std::size_t m_nb_samples {0};
      std::size_t m_nb_parts {0};
      std::size_t m_smer_size {0};
      std::size_t m_z_size {0};
      std::size_t m_width {0};

      repart_type m_repart {nullptr};
      hw_type m_hw {nullptr};
      std::size_t m_msize {0};

      std::vector<query_response_t> m_responses;
      std::vector<qpart_type> m_smers;
  };

}


#endif /* end of include guard: QUERY_HPP_HIJEZMAU */
