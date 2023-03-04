#include <thread>
#include "common.hpp"

namespace kmq {

  void add_common_options(bc::cmd_t cmd, kmq_options_t options, bool with_threads, std::size_t n)
  {
    cmd->add_group("common", "");

    if (with_threads)
    {
      cmd->add_param("-t/--threads", "Number of threads.")
        ->def(std::to_string(n))
        ->meta("INT")
        ->setter(options->nb_threads)
        ->checker(bc::check::is_number);
    }

    cmd->add_param("-h/--help", "Show this message and exit.")
      ->as_flag()
      ->action(bc::Action::ShowHelp);
    cmd->add_param("--version", "Show version and exit.")
      ->as_flag()
      ->action(bc::Action::ShowVersion);
    cmd->add_param("-v/--verbose", "Verbosity level [debug|info|warning|error].")
      ->meta("STR")
      ->def("info")
      ->checker(bc::check::f::in("debug|info|warning|error"))
      ->setter(options->verbosity);
  }

}
