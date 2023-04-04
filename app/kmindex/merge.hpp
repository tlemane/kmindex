#ifndef MERGE_HPP_RTMU4VNU
#define MERGE_HPP_RTMU4VNU

#include <vector>
#include <string>

#include <kmindex/index/merge.hpp>

#include "common.hpp"

namespace kmq {

  struct kmq_merge_options : kmq_options
  {
    std::vector<std::string> to_merge;
    std::string name;
    bool remove {false};
    std::string new_path;

    std::string rename;
    rename_mode mode {rename_mode::none};
  };

  using kmq_merge_options_t = std::shared_ptr<struct kmq_merge_options>;

  kmq_options_t kmq_merge_cli(parser_t parser, kmq_merge_options_t options);

  void main_merge(kmq_options_t opt);
}

#endif /* end of include guard: MERGE_HPP_RTMU4VNU */

