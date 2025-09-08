#ifndef REGISTER_HPP_X3UGAJR1
#define REGISTER_HPP_X3UGAJR1

#include "common.hpp"

namespace kmq {

  struct kmq_register_options : kmq_options
  {
    std::string index_name;
    std::string index_path;
    std::string from_file;
  };

  using kmq_register_options_t = std::shared_ptr<struct kmq_register_options>;

  kmq_options_t kmq_register_cli(parser_t parser, kmq_register_options_t);

  void main_register(kmq_options_t opt);
}


#endif /* end of include guard: REGISTER_HPP_X3UGAJR1 */
