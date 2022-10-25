#include <cstring>
#include <kmindex/threadpool.hpp>
#include <kmindex/mer.hpp>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/kindex.hpp>

namespace kmq {

  partition::partition(const std::string& matrix_path, std::size_t nb_samples, std::size_t width)
    : m_nb_samples(nb_samples), m_bytes(((nb_samples * width) + 7) / 8)
  {
    m_fd = open(matrix_path.c_str(), O_RDONLY);
    m_mapped = mio::mmap_source(m_fd, 0, mio::map_entire_file);
  }

  partition::~partition()
  {
    close(m_fd);
  }

  void partition::query(std::uint64_t pos, std::uint8_t* dest)
  {
    //std::unique_lock<std::mutex> lock(m_mutex);
    std::memcpy(dest, m_mapped.begin() + (m_bytes * pos) + 49, m_bytes);
  }

  kindex::kindex() {}

  kindex::kindex(const index_infos& i)
    : m_infos(i)
  {
    for (std::size_t i = 0; i < m_infos.nb_partitions(); ++i)
    {
      m_partitions.push_back(std::make_unique<partition>(m_infos.get_partition(i), m_infos.nb_samples()));
    }
  }

  std::string kindex::name() const { return m_infos.name(); }
  std::string kindex::directory() const { return m_infos.path(); }

  query_result kindex::resolve(query& q)
  {
    smer_hasher sh(m_infos.get_repartition(), m_infos.get_hash_w(), m_infos.minim_size());

    for (auto& e : q.iterate(m_infos.smer_size(), &sh))
    {
      m_partitions[e.p]->query(e.h, q.response_block(e.i));
    }

    return query_result(&q, m_infos.nb_samples());
  }
}
