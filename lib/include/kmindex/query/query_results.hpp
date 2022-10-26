#ifndef QUERY_RESULTS_HPP_VLPMAIEE
#define QUERY_RESULTS_HPP_VLPMAIEE

#include <mutex>
#include <kmindex/query/query.hpp>

namespace kmq {

  class query_result
  {
    public:

      query_result(query* q, std::size_t nb_samples);

    public:

      void compute_ratios();

      std::size_t nbk() const;

      const std::vector<std::uint32_t>& counts() const;

      const std::vector<double>& ratios() const;

      const std::string& name() const;

      double threshold() const;

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
    using vec_t = std::vector<query_result>;
    using iterator = vec_t::iterator;
    using const_iterator = vec_t::const_iterator;

    public:
      query_result_agg() = default;

      void add(query_result&& r);

      std::size_t size() const;

      const_iterator begin() const;

      const_iterator end() const;

    private:
      std::mutex m_mutex;
      std::vector<query_result> m_results;
  };

}

#endif /* end of include guard: QUERY_RESULTS_HPP_VLPMAIEE */
