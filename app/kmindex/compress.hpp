#ifndef COMPRESS_HPP_1756992492
#define COMPRESS_HPP_1756992492

#include "common.hpp"

namespace kmq {

  struct kmq_compress_options : kmq_options
  {
    std::string index_name;

    bool reorder {false};
    std::size_t block_size {8}; // in MB
    bool delete_old {false};

    std::size_t sampling {20000};
    std::size_t column_blocks {1};
    bool check {false}; 
  };

  using kmq_compress_options_t = std::shared_ptr<struct kmq_compress_options>;

  kmq_options_t kmq_compress_cli(parser_t parser, kmq_compress_options_t);

  void main_compress(kmq_options_t opt);
}


#endif /* end of include guard: COMPRESS_HPP_1756992492 */