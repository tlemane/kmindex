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
      kindex(const index_infos& i);

      void init(std::size_t p);
      void unmap(std::size_t p);

    public:
      std::string name() const;
      std::string directory() const;

      void solve(batch_query& bq)
      {
        for (std::size_t p = 0; p < m_infos.nb_partitions(); p++)
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

      index_infos& infos();
    private:
      index_infos m_infos;
      std::vector<std::unique_ptr<partition>> m_partitions;
      std::vector<spinlock> m_mutexes;
  };
}



#endif /* end of include guard: INDEX_HPP_FJYOTLJN */
