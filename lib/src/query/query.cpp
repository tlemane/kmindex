#include <kmindex/query/query.hpp>

namespace kmq {

  query_response::query_response(std::size_t n, std::size_t nbits, std::size_t width)
    : m_block_size(((nbits * width) + 7) / 8)
  {
    m_responses.resize(n * m_block_size, 0);
  }

  std::size_t query_response::block_size() const
  {
    return m_block_size;
  }

  std::uint8_t* query_response::get(std::size_t mer_pos)
  {
    return m_responses.data() + (mer_pos * m_block_size);
  }

  query::query(const std::string& name,
        const std::string& seq,
        std::size_t smer_size,
        std::size_t z_size,
        std::size_t nb_samples,
        double threshold,
        std::size_t width)
    : m_name(name),
      m_seq(seq),
      m_size(m_seq.size() - smer_size + 1),
      m_ksize(m_seq.size() - (smer_size + z_size) + 1),
      m_zsize(z_size),
      m_threshold(threshold)
  {
    m_responses = std::make_unique<query_response>(size(), nb_samples, width);
  }

  std::uint8_t* query::response_block(std::size_t mer_pos)
  {
    return m_responses->get(mer_pos);
  }

  std::size_t query::block_size() const
  {
    return m_responses->block_size();
  }

  std::size_t query::size() const
  {
    return m_size;
  }

  std::size_t query::ksize() const
  {
    return m_ksize;
  }

  std::size_t query::zsize() const
  {
    return m_zsize;
  }

  std::string query::name() const
  {
    return m_name;
  }

  double query::threshold() const
  {
    return m_threshold;
  }

  smer_iterator query::iterate(std::size_t smer_size, smer_hasher* h)
  {
    return smer_iterator(m_seq, smer_size, h);
  }

}
