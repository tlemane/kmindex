#ifndef QUERY_RESULTS_HPP_VLPMAIEE
#define QUERY_RESULTS_HPP_VLPMAIEE

#include <mutex>
#include <kmindex/query/query.hpp>


namespace kmq {

  class query_result
  {
    public:

      query_result(query* q, std::size_t nb_samples)
        : m_q(q), m_z(q->zsize())
      {
        m_ratios.resize(nb_samples, 0);
        m_counts.resize(nb_samples, 0);
        m_name = m_q->name();
        m_threshold = m_q->threshold();
        m_nbk = m_q->ksize();

        compute_ratios();
      }

    public:
      void compute_ratios()
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

      std::size_t nbk() const { return m_nbk; }
      const std::vector<std::uint32_t>& counts() const { return m_counts; }
      const std::vector<double>& ratios() const { return m_ratios; }
      const std::string& name() const { return m_name; }
      double threshold() const { return m_threshold; }

    private:
      query* m_q;
      std::size_t m_z;
      std::vector<double> m_ratios;
      std::vector<std::uint32_t> m_counts;
      double m_threshold;
      std::string m_name;
      std::size_t m_nbk;
  };

  class query_result_agg
  {
    public:
    void add(query_result&& r)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_results.push_back(std::move(r));
    }

    std::size_t size() const { return m_results.size(); }
    auto begin() const { return m_results.begin(); }
    auto end() const { return m_results.end(); }

    private:
      std::mutex m_mutex;
      std::vector<query_result> m_results;
  };

}

#endif /* end of include guard: QUERY_RESULTS_HPP_VLPMAIEE */
