#include <cstring>
#include <kmindex/threadpool.hpp>
#include <kmindex/mer.hpp>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/kindex.hpp>
#include <sys/mman.h>

namespace kmq {

  partition::partition(const std::string& matrix_path, std::size_t nb_samples, std::size_t width)
    : m_nb_samples(nb_samples), m_bytes(((nb_samples * width) + 7) / 8)
  {
    m_fd = open(matrix_path.c_str(), O_RDONLY);
    m_mapped = mio::mmap_source(m_fd, 0, mio::map_entire_file);
//    posix_madvise(&m_mapped[0], m_mapped.length(), POSIX_MADV_SEQUENTIAL);
  }

  partition::~partition()
  {
    m_mapped.unmap();
    close(m_fd);
  }

  void partition::query(std::uint64_t pos, std::uint8_t* dest)
  {
    std::memcpy(dest, m_mapped.begin() + (m_bytes * pos) + 49, m_bytes);
  }

  kindex::kindex() {}

  kindex::kindex(const index_infos& i, bool cache)
    : m_infos(i), m_mutexes(i.nb_partitions()), m_cache(cache)
  {
    m_partitions.resize(m_infos.nb_partitions());

    if (m_cache)
    {
      for (std::size_t p = 0; p < m_infos.nb_partitions(); ++p)
      {
        init(p);
      }
    }
  }

  kindex::~kindex()
  {
    if (m_cache)
    {
      for (std::size_t p = 0; p < m_infos.nb_partitions(); ++p)
      {
        unmap(p);
      }
    }
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

  index_infos& kindex::infos()
  {
    return m_infos;
  }
}
