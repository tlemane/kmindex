#ifndef QUERY_SUM_HPP_1757148870
#define QUERY_SUM_HPP_1757148870

#include "common.hpp"

namespace kmq {

  struct kmq_sum_query_options : kmq_options
  {
    std::string query_file;
    std::vector<std::string> index_names;
    std::size_t z {0};
    std::string output;
    double sk_threshold {0.25};
  };

  using kmq_sum_query_options_t = std::shared_ptr<struct kmq_sum_query_options>;

  kmq_options_t kmq_sum_query_cli(parser_t parser, kmq_sum_query_options_t);

  void main_sum_query(kmq_options_t opt);
}

#endif /* end of include guard: QUERY_SUM_HPP_1757148870 */