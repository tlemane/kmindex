#ifndef INDEX_HPP_FJYOTLJN
#define INDEX_HPP_FJYOTLJN

#include <memory>
//#include <kmindex/query/query.hpp>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmtricks/repartition.hpp>

#include <mio/mmap.hpp>

namespace kmq {

  class partition
  {
    public:
      partition(const std::string& matrix_path, std::size_t nb_samples, std::size_t width = 1);

      ~partition();

      void query(std::uint64_t pos, std::uint8_t* dest);

    private:
      int m_fd {0};
      mio::mmap_source m_mapped;
      std::size_t m_nb_samples {0};
      std::size_t m_bytes {0};
      std::mutex m_mutex;
  };

  class kindex
  {
    public:

      kindex();
      kindex(const index_infos& i);

      auto& partitions() { return m_partitions; }
      void init(std::size_t p);
      void unmap(std::size_t p);

    public:
      std::string name() const;
      std::string directory() const;

      query_result resolve(query& q);
      query_result resolve(query& q, std::size_t nbt);

      index_infos& infos() { return m_infos; }
    private:
      index_infos m_infos;
      std::vector<std::unique_ptr<partition>> m_partitions;

  };
}



#endif /* end of include guard: INDEX_HPP_FJYOTLJN */
