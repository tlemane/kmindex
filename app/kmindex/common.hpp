#ifndef COMMON_HPP_THWSML1X
#define COMMON_HPP_THWSML1X

#include <string>
#include <memory>
#include <cstdint>

#include <bcli/bcli.hpp>

namespace kmq {

  enum class kmq_commands
  {
    kmq_build,
    kmq_query,
    kmq_register,
    kmq_merge,
    kmq_index_infos
  };

  struct kmq_options
  {
    std::string verbosity;
    std::size_t nb_threads;
    std::string global_index_path;
  };

  using kmq_options_t = std::shared_ptr<struct kmq_options>;

  using parser_t = std::shared_ptr<bc::Parser<1>>;

  void add_common_options(bc::cmd_t cmd, kmq_options_t options, bool with_threads = false);

}


#endif /* end of include guard: COMMON_HPP_THWSML1X */
