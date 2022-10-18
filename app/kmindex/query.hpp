#ifndef QUERY_HPP_ZDULHSN1
#define QUERY_HPP_ZDULHSN1

#include "common.hpp"

namespace kmq {

  struct kmq_query_options : kmq_options
  {
    std::string index;
    std::string index_name;
    std::string output;
    std::string input;

    std::size_t z {0};
    double sk_threshold {0};
  };

  using kmq_query_options_t = std::shared_ptr<struct kmq_query_options>;

  kmq_options_t kmq_query_cli(parser_t parser, kmq_query_options_t options);

  void main_query(kmq_options_t opt);
}

#endif /* end of include guard: QUERY_HPP_ZDULHSN1 */
