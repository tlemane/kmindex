#ifndef BUILD_HPP_4CDHAOMO
#define BUILD_HPP_4CDHAOMO

#include <vector>
#include "common.hpp"

namespace kmq {

  struct kmq_build_options : kmq_options
  {
    std::string fof;
    std::string directory;
    std::string from;
    std::string name;
    std::string km_path;

    std::size_t kmer_size {0};
    std::size_t minim_size {0};
    std::size_t hard_min {0};
    std::size_t bloom_size {0};
    std::size_t nb_cell {0};
    std::size_t nb_partitions {0};
    std::size_t bw {2};
    bool no_check {false};
    bool cpr {false};
  };

  using kmq_build_options_t = std::shared_ptr<struct kmq_build_options>;

  kmq_options_t kmq_build_cli(parser_t parser, kmq_build_options_t options);

  void main_build(kmq_options_t opt);
}

#endif /* end of include guard: BUILD_HPP_4CDHAOMO */
