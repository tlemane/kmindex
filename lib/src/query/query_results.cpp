#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/query/format.hpp>

namespace kmq {

  query_result::query_result(query_response_t&& qr, std::size_t z, const index_infos& infos)
    : m_qr(std::move(qr)), m_z(z), m_infos(infos), m_nbk(m_qr->nbk() - z)
  {
    m_ratios.resize(m_infos.nb_samples(), 0);
    m_counts.resize(m_infos.nb_samples(), 0);
    compute_ratios();
  }

  void query_result::compute_ratios()
  {
    const uint8_t* data = m_qr->get(0);

    std::vector<uint8_t> kres(m_qr->block_size(), 255);
    std::vector<std::uint32_t> count(m_ratios.size(), 0);

    std::size_t block_size_z = m_qr->block_size() * m_z;

    for (std::size_t i = 0; i < m_nbk * m_qr->block_size(); i += m_qr->block_size())
    {
      for (std::size_t j = i; j <= i + block_size_z; j += m_qr->block_size())
      {
        for(std::size_t k = 0; k < m_qr->block_size(); ++k)
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
      m_ratios[i] = m_counts[i] / static_cast<double>(m_nbk);
    }

    m_qr->free();
  }

  std::size_t query_result::nbk() const
  {
    return m_nbk;
    // return m_qr->nbk() - m_z;
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
    return m_qr->name();
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

  void query_result_agg::output(const index_infos& infos,
                                const std::string& output_dir,
                                enum format f,
                                const std::string& qname,
                                double threshold)
  {
    fs::create_directory(output_dir);

    std::ofstream out(
      fmt::format("{}/{}.{}", output_dir, infos.name(), f == format::matrix ? "tsv" : "json"));

    auto formatter = make_formatter(f, threshold);

    if (qname.size() > 0)
    {
      formatter->merge_format(infos, qname, m_results, out);
    }
    else
    {
      for (auto& r : m_results)
      {
        formatter->format(infos, r, out);
      }
    }

  }

}
