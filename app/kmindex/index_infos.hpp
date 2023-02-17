#ifndef INDEX_INFOS_HPP_15ZWXKGP
#define INDEX_INFOS_HPP_15ZWXKGP

#include <vector>
#include "common.hpp"

namespace kmq {

  struct kmq_infos_options : kmq_options
  {
  };

  using kmq_infos_options_t = std::shared_ptr<struct kmq_infos_options>;

  kmq_options_t kmq_infos_cli(parser_t parser, kmq_infos_options_t options);

  void main_infos(kmq_options_t opt);
}

#endif /* end of include guard: INDEX_INFOS_HPP_15ZWXKGP */
