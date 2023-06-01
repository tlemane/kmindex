#ifndef INDEX_HPP_FJYOTLJN
#define INDEX_HPP_FJYOTLJN

#include <memory>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/spinlock.hpp>
#include <mio/mmap.hpp>


#include <iostream>

namespace kmq {

  class partition
  {
    public:
      partition(const std::string& matrix_path, std::size_t nb_samples, std::size_t width);

      ~partition();

      void query(std::uint64_t pos, std::uint8_t* dest);

    private:
      int m_fd {0};
      mio::mmap_source m_mapped;
      std::size_t m_nb_samples {0};
      std::size_t m_bytes {0};
  };

  class kindex
  {
    public:

      kindex();
      ~kindex();
      kindex(const index_infos& i, bool cache = false);

      void init(std::size_t p);
      void unmap(std::size_t p);

    public:
      std::string name() const;
      std::string directory() const;

      void solve(batch_query& bq)
      {
        std::vector<std::size_t> order; order.reserve(m_infos.nb_partitions());
        for (std::size_t p = 0; p < m_infos.nb_partitions(); p++)
          order.push_back(p);
#ifndef __APPLE__
        std::random_shuffle(std::begin(order), std::end(order));
#endif
        for (auto const& p : order)
          solve_one(bq, p);
      }

      void solve_one(batch_query& bq, std::size_t p)
      {
        auto& smers = bq.partition(p);
        auto& responses = bq.response();

        std::sort(std::begin(smers), std::end(smers));

        std::unique_lock<spinlock> lock(m_mutexes[p]);
        init(p);
        for (auto& [mer, qid] : smers)
        {
          m_partitions[p]->query(mer.h, responses[qid]->get(mer.i));
        }
        m_partitions[p] = nullptr;
      }

      void solve_cache(batch_query& bq)
      {
        std::vector<std::size_t> order; order.reserve(m_infos.nb_partitions());
        for (std::size_t p = 0; p < m_infos.nb_partitions(); p++)
          order.push_back(p);
#ifndef __APPLE__
        std::random_shuffle(std::begin(order), std::end(order));
#endif
        for (auto const& p : order)
          solve_one_cache(bq, p);
      }

      void solve_one_cache(batch_query& bq, std::size_t p)
      {
        auto& smers = bq.partition(p);
        auto& responses = bq.response();

        std::sort(std::begin(smers), std::end(smers));

        std::unique_lock<spinlock> lock(m_mutexes[p]);
        for (auto& [mer, qid] : smers)
        {
          m_partitions[p]->query(mer.h, responses[qid]->get(mer.i));
        }
      }

      void solve_batch(batch_query& bq)
      {
        if (m_cache)
          solve_cache(bq);
        else
          solve(bq);
      }

      index_infos& infos();
    private:
      index_infos m_infos;
      std::vector<std::unique_ptr<partition>> m_partitions;
      std::vector<spinlock> m_mutexes;
      bool m_cache {false};
  };
}



#endif /* end of include guard: INDEX_HPP_FJYOTLJN */
