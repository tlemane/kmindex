#ifndef QUERY_HPP_HIJEZMAU
#define QUERY_HPP_HIJEZMAU

#include <cstdint>
#include <atomic>
#include <string_view>
#include <cassert>

#ifndef KMTRICKS_PUBLIC
  #define KMTRICKS_PUBLIC
#endif

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

    public:
      batch_query(std::size_t nb_samples, std::size_t nb_partitions, std::size_t smer_size, std::size_t z_size, std::size_t width, smer_hasher* hasher)
        : m_nb_samples(nb_samples),
          m_nb_parts(nb_partitions),
          m_smer_size(smer_size),
          m_z_size(z_size),
          m_width(width),
          m_hasher(hasher),
          m_pmutexes(m_nb_parts),
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
        std::uint32_t qid = insert(name, seq.size() - m_smer_size + 1);

        std::vector<qpart_type> tmp(m_nb_parts);
        for (auto& mer : smer_iterator(seq, m_smer_size, m_hasher))
        {
          tmp[mer.p].emplace_back(mer, qid);
          //m_smers[mer.p].emplace_back(mer, qid);
        }

        {
          for (std::size_t p = 0; p < tmp.size(); ++p)
          {
            std::unique_lock<std::mutex> _(m_pmutexes[p]);
            m_smers[p].insert(std::end(m_smers[p]), std::begin(tmp[p]), std::end(tmp[p]));
          }
        }
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

    private:
      std::uint32_t insert(const std::string& name, std::size_t nb_smers)
      {
        std::unique_lock<std::mutex> _(m_mutex);
        m_responses.push_back(
            std::make_unique<query_response>(std::move(name), nb_smers, m_nb_samples, m_width));
        return m_responses.size() - 1;
      }

    private:
      std::size_t m_nb_samples {0};
      std::size_t m_nb_parts {0};
      std::size_t m_smer_size {0};
      std::size_t m_z_size {0};
      std::size_t m_width {0};

      smer_hasher* m_hasher {nullptr};

      std::mutex m_mutex;
      std::vector<std::mutex> m_pmutexes;

      std::vector<query_response_t> m_responses;
      std::vector<qpart_type> m_smers;
  };

}


#endif /* end of include guard: QUERY_HPP_HIJEZMAU */
