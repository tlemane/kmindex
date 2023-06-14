#ifndef DUMP_HPP_DQKREG5J
#define DUMP_HPP_DQKREG5J

#include <kmindex/query/format.hpp>
#include "common.hpp"

namespace kmq {

  struct kmq_dump_options : kmq_options
  {
    enum format format;
    std::string input;
    std::string output;
  };

  using kmq_dump_options_t = std::shared_ptr<struct kmq_dump_options>;

  kmq_options_t kmq_dump_cli(parser_t parser, kmq_dump_options_t options);

  void main_dump(kmq_options_t opt);
}

#endif /* end of include guard: DUMP_HPP_DQKREG5J */
