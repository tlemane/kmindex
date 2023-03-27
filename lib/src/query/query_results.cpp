#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/query/format.hpp>

#include <bitpacker/bitpacker.hpp>
#include <nonstd/span.hpp>

#include <iostream>

namespace kmq {

  query_result::query_result(query_response_t&& qr, std::size_t z, const index_infos& infos, bool pos)
    : m_qr(std::move(qr)), m_z(z), m_infos(infos), m_nbk(m_qr->nbk() - z)
  {
    m_ratios.resize(m_infos.nb_samples(), 0);
    m_counts.resize(m_infos.nb_samples(), 0);

    if (pos)
      m_positions.resize(m_infos.nb_samples());

    if (m_infos.bw() > 1)
    {
      if (pos)
        compute_abs_pos();
      else
        compute_abs();
    }
    else
    {
      if (pos)
        compute_ratios_pos();
      else
        compute_ratios();
    }
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

  void query_result::compute_ratios_pos()
  {
    const uint8_t* data = m_qr->get(0);

    std::vector<uint8_t> kres(m_qr->block_size(), 255);
    std::vector<std::uint32_t> count(m_ratios.size(), 0);

    std::size_t block_size_z = m_qr->block_size() * m_z;

    for (auto& v : m_positions)
      v.reserve(m_nbk);

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
        auto b = static_cast<bool>(BITCHECK(kres, s));
        m_counts[s] += static_cast<std::uint8_t>(b);
        m_positions[s].push_back(static_cast<std::uint8_t>(b));
      }

      std::fill(kres.begin(), kres.end(), 255);
    }

    for (std::size_t i = 0; i < m_ratios.size(); ++i)
    {
      m_ratios[i] = m_counts[i] / static_cast<double>(m_nbk);
    }

    m_qr->free();
  }

  void query_result::compute_abs_pos()
  {
    const uint8_t* data = m_qr->get(0);

    std::vector<std::uint32_t> kres_abs(m_counts.size(), std::numeric_limits<std::uint32_t>::max());
    std::fill(m_counts.begin(), m_counts.end(), std::numeric_limits<std::uint32_t>::min());

    std::size_t block_size_z = m_qr->block_size() * m_z;

    for (auto& v : m_positions)
      v.reserve(m_nbk);

    // for each k-mers
    for (std::size_t i = 0; i < m_nbk * m_qr->block_size(); i += m_qr->block_size())
    {
      // for each s-mers in the k-mer
      for (std::size_t j = i; j <= i + block_size_z; j += m_qr->block_size())
      {
        auto s = nonstd::span<const std::uint8_t>(&data[j], m_qr->block_size());
        for(std::size_t k = 0, l = 0; k < m_counts.size(); ++k, l+=m_infos.bw())
        {
          kres_abs[k] = std::min(bitpacker::extract<std::uint32_t>(s, l, m_infos.bw()), kres_abs[k]);
        }
      }

      // at this point, kres is a vector of abundances of a k-mer (min of s-mers) across samples
      for (std::size_t s = 0; s < m_ratios.size(); ++s)
      {
        m_counts[s] += kres_abs[s];
        m_positions[s].push_back(kres_abs[s]);
      }

      std::fill(kres_abs.begin(), kres_abs.end(), std::numeric_limits<std::uint32_t>::max());
    }

    for (std::size_t i = 0; i < m_ratios.size(); ++i)
    {
      m_counts[i] = m_counts[i] / m_nbk;
    }

  }

  void query_result::compute_abs()
  {
    const uint8_t* data = m_qr->get(0);

    std::vector<std::uint32_t> kres_abs(m_counts.size(), std::numeric_limits<std::uint32_t>::max());
    std::fill(m_counts.begin(), m_counts.end(), std::numeric_limits<std::uint32_t>::min());

    std::size_t block_size_z = m_qr->block_size() * m_z;

    // for each k-mers
    for (std::size_t i = 0; i < m_nbk * m_qr->block_size(); i += m_qr->block_size())
    {
      // for each s-mers in the k-mer
      for (std::size_t j = i; j <= i + block_size_z; j += m_qr->block_size())
      {
        auto s = nonstd::span<const std::uint8_t>(&data[j], m_qr->block_size());
        for(std::size_t k = 0, l = 0; k < m_counts.size(); ++k, l+=m_infos.bw())
        {
          kres_abs[k] = std::min(bitpacker::extract<std::uint32_t>(s, l, m_infos.bw()), kres_abs[k]);
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
      m_counts[i] = m_counts[i] / m_nbk;
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

  const std::vector<std::vector<std::uint8_t>>& query_result::positions() const
  {
    return m_positions;
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

  const query_result_agg::vec_t& query_result_agg::results() const
  {
    return m_results;
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

    auto formatter = make_formatter(f, threshold, infos.bw());

    if (qname.size() > 0)
    {
      formatter->merge_format(infos, qname, m_results, out);
    }
    else
    {
      formatter->write_headers(out, infos);
      for (auto& r : m_results)
      {
        formatter->format(infos, r, out);
      }
    }

  }

}
