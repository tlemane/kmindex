#ifndef INDEX_SUM_HPP_1757091271
#define INDEX_SUM_HPP_1757091271

#include "common.hpp"

namespace kmq {

  struct kmq_sum_index_options : kmq_options
  {
    std::string index_name;
    double correction {0.0};
    bool estimate_correction {false};
    std::size_t nbk {10000};
  };

  using kmq_sum_index_options_t = std::shared_ptr<struct kmq_sum_index_options>;

  kmq_options_t kmq_sum_index_cli(parser_t parser, kmq_sum_index_options_t);

  void main_sum_index(kmq_options_t opt);
}

#endif /* end of include guard: INDEX_SUM_HPP_1757091271 */