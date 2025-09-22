#ifndef COMMON_HPP_THWSML1X
#define COMMON_HPP_THWSML1X

#include <string>
#include <memory>
#include <cstdint>
#include <thread>

#include <bcli/bcli.hpp>

namespace kmq {

  inline std::string random_sequence(std::size_t n)
  {
    static const char alphanum[] =
      "ACGT";

    std::string s;
    s.reserve(n);

    for (std::size_t i = 0; i < n; ++i)
    {
      s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return s;
  }

  enum class kmq_commands
  {
    kmq_build,
    kmq_query,
    kmq_register,
    kmq_merge,
    kmq_index_infos,
    kmq_compress,
    kmq_sum_index,
    kmq_sum_query
  };

  struct kmq_options
  {
    std::string verbosity;
    std::size_t nb_threads;
    std::string global_index_path;
  };

  using kmq_options_t = std::shared_ptr<struct kmq_options>;

  using parser_t = std::shared_ptr<bc::Parser<1>>;

  void add_common_options(bc::cmd_t cmd,
                          kmq_options_t options,
                          bool with_threads = false,
                          std::size_t n = std::thread::hardware_concurrency());

}


#endif /* end of include guard: COMMON_HPP_THWSML1X */
