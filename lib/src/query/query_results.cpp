#include <kmindex/query/query_results.hpp>

namespace kmq {

  query_result::query_result(query* q, std::size_t nb_samples)
    : m_q(q), m_z(q->zsize())
  {
    m_ratios.resize(nb_samples, 0);
    m_counts.resize(nb_samples, 0);
    m_name = m_q->name();
    m_threshold = m_q->threshold();
    m_nbk = m_q->ksize();

    compute_ratios();
  }

  void query_result::compute_ratios()
  {
    const uint8_t* data = m_q->response_block(0);

    std::vector<uint8_t> kres(m_q->block_size(), 255);
    std::vector<std::uint32_t> count(m_ratios.size(), 0);

    std::size_t block_size_z = m_q->block_size() * m_z;

    for (std::size_t i = 0; i < m_q->ksize() * m_q->block_size(); i += m_q->block_size())
    {
      for (std::size_t j = i; j <= i + block_size_z; j += m_q->block_size())
      {
        for(std::size_t k = 0; k < m_q->block_size(); ++k)
        {
          kres[k] &= data[j+k];
        }
      }

      for (std::size_t s = 0; s < m_ratios.size(); ++s)
      {
        m_counts[s] += static_cast<bool>(BITCHECK(kres, s));
      }

      std::fill(kres.begin(), kres.end(), 255);
    }

    for (std::size_t i = 0; i < m_ratios.size(); ++i)
    {
      m_ratios[i] = m_counts[i] / static_cast<double>(m_q->ksize());
    }
  }

  std::size_t query_result::nbk() const
  {
    return m_nbk;
  }

  const std::vector<std::uint32_t>& query_result::counts() const
  {
    return m_counts;
  }

  const std::vector<double>& query_result::ratios() const
  {
    return m_ratios;
  }

  const std::string& query_result::name() const
  {
    return m_name;
  }

  double query_result::threshold() const
  {
    return m_threshold;
  }

  void query_result_agg::add(query_result&& r)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_results.push_back(std::move(r));
  }

  std::size_t query_result_agg::size() const
  {
    return m_results.size();
  }

  query_result_agg::const_iterator query_result_agg::begin() const
  {
    return m_results.begin();
  }

  query_result_agg::const_iterator query_result_agg::end() const
  {
    return m_results.end();
  }

}