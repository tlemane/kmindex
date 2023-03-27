#ifndef QUERY_RESULTS_HPP_VLPMAIEE
#define QUERY_RESULTS_HPP_VLPMAIEE

#include <mutex>
#include <kmindex/query/query.hpp>

namespace kmq {

  class index_infos;

  enum class format;

  class query_result
  {
    public:

      query_result(query_response_t&& qr, std::size_t z, const index_infos& info, bool pos = false);

    public:

      void compute_ratios();

      void compute_abs();

      void compute_ratios_pos();

      void compute_abs_pos();

      std::size_t nbk() const;

      const std::vector<std::uint32_t>& counts() const;

      const std::vector<double>& ratios() const;

      const std::vector<std::vector<std::uint8_t>>& positions() const;

      const std::string& name() const;

      double threshold() const;

    private:
      std::vector<double> m_ratios;
      std::vector<std::uint32_t> m_counts;
      std::vector<std::vector<std::uint8_t>> m_positions;
      query_response_t m_qr;
      std::size_t m_z;
      const index_infos& m_infos;
      std::uint32_t m_nbk;
  };

  class query_result_agg
  {
    using vec_t = std::vector<query_result>;
    using iterator = vec_t::iterator;
    using const_iterator = vec_t::const_iterator;

    public:
      query_result_agg() = default;

      void add(query_result&& r);

      std::size_t size() const;

      const_iterator begin() const;

      const_iterator end() const;

      const vec_t& results() const;

      void output(const index_infos& infos,
                  const std::string& output_dir,
                  enum format f,
                  const std::string& qname,
                  double threshold);

    private:
      std::mutex m_mutex;
      std::vector<query_result> m_results;
  };

}

#endif /* end of include guard: QUERY_RESULTS_HPP_VLPMAIEE */
