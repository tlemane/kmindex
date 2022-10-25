#ifndef SERVER_HPP_7KR2PWBZ
#define SERVER_HPP_7KR2PWBZ

#include <memory>
#include <bcli/bcli.hpp>

namespace kmq {

  using parser_t = std::shared_ptr<bc::Parser<0>>;

  struct kmq_server_options
  {
    std::string verbosity;
    std::string index_path;
    std::size_t port;
    std::string address;
    std::string log_directory;
  };

  using kmq_server_options_t = std::shared_ptr<struct kmq_server_options>;

  kmq_server_options_t kmq_server_cli(parser_t parser, kmq_server_options_t options);

  void main_server(kmq_server_options_t opt);
}

#endif /* end of include guard: SERVER_HPP_7KR2PWBZ */
