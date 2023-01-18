#include <bitpacker/bitpacker.hpp>
#include <kmindex/query/query_results.hpp>
#include <nonstd/span.hpp>

namespace kmq
{

  query_result::query_result(query* q, std::size_t nb_samples)
      : m_q(q), m_z(q->zsize())
  {
    m_ratios.resize(nb_samples, 0);
    m_counts.resize(nb_samples, 0);
    m_name = m_q->name();
    m_threshold = m_q->threshold();
    m_nbk = m_q->ksize();

    if (m_q->width() > 1)
      compute_abs();
    else
      compute_ratios();
  }

  void query_result::compute_ratios()
  {
    const uint8_t* data = m_q->response_block(0);

    std::vector<uint8_t> kres(m_q->block_size(), 255);

    std::size_t block_size_z = m_q->block_size() * m_z;

    for (std::size_t i = 0; i < m_q->ksize() * m_q->block_size(); i += m_q->block_size())
    {
      for (std::size_t j = i; j <= i + block_size_z; j += m_q->block_size())
      {
        for (std::size_t k = 0; k < m_q->block_size(); ++k)
        {
          kres[k] &= data[j + k];
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

  void query_result::compute_abs()
  {
    const uint8_t* data = m_q->response_block(0);

    std::vector<std::uint32_t> kres_abs(m_counts.size(), std::numeric_limits<std::uint32_t>::max());
    std::fill(m_counts.begin(), m_counts.end(), std::numeric_limits<std::uint32_t>::min());

    std::size_t block_size_z = m_q->block_size() * m_z;

    // for each k-mers
    for (std::size_t i = 0; i < m_q->ksize() * m_q->block_size(); i += m_q->block_size())
    {
      // for each s-mers in the k-mer
      for (std::size_t j = i; j <= i + block_size_z; j += m_q->block_size())
      {
        auto s = nonstd::span<const std::uint8_t>(&data[j], m_q->block_size());
        for (std::size_t k = 0, l = 0; k < m_counts.size(); ++k, l += m_q->width())
        {
          kres_abs[k] = std::min(bitpacker::extract<std::uint32_t>(s, l, m_q->width()), kres_abs[k]);
        }
      }

      // at this point, kres is a vector of abundances of a k-mer (min of s-mers) across samples
      for (std::size_t s = 0; s < m_ratios.size(); ++s)
      {
        m_counts[s] += kres_abs[s];
      }

      std::fill(kres_abs.begin(), kres_abs.end(), std::numeric_limits<std::uint32_t>::max());
    }

    for (std::size_t i = 0; i < m_ratios.size(); ++i)
    {
      m_counts[i] = m_counts[i] / m_q->ksize();
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

}  // namespace kmq
