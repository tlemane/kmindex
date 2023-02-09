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
    std::memcpy(dest, m_mapped.begin() + (m_bytes * pos) + 49, m_bytes);
  }

  kindex::kindex() {}

  kindex::kindex(const index_infos& i)
    : m_infos(i)
  {
    m_partitions.resize(m_infos.nb_partitions());
  }

  void kindex::init(std::size_t p)
  {
    m_partitions[p] = std::make_unique<partition>(m_infos.get_partition(p), m_infos.nb_samples(), m_infos.bw());
  }

  void kindex::unmap(std::size_t p)
  {
    m_partitions[p] = nullptr;
  }

  std::string kindex::name() const
  {
    return m_infos.name();
  }

  std::string kindex::directory() const
  {
    return m_infos.path();
  }

  query_result kindex::resolve(query& q)
  {
    smer_hasher sh(m_infos.get_repartition(), m_infos.get_hash_w(), m_infos.minim_size());

    std::vector<std::vector<smer>> smers(m_infos.nb_partitions());

    for (auto& e : q.iterate(m_infos.smer_size(), &sh))
    {
      smers[e.p].push_back(e);
    }

    {
      std::unique_lock<spinlock> lock(m_mutex);
      for (auto& v : smers)
      {
        if (v.size())
        {
          init(v[0].p);
          for (auto& e : v)
          {
            m_partitions[e.p]->query(e.h, q.response_block(e.i));
          }
          unmap(v[0].p);

          std::vector<smer> d;
          v.swap(d);
        }
      }
    }

    return query_result(&q, m_infos.nb_samples());
  }

  index_infos& kindex::infos()
  {
    return m_infos;
  }
}
