#include <thread>
#include "common.hpp"

namespace kmq {

  void add_common_options(bc::cmd_t cmd, kmq_options_t options, bool with_threads)
  {
    cmd->add_group("common", "");

    if (with_threads)
    {
      cmd->add_param("-t/--threads", "number of threads.")
        ->def(std::to_string(std::thread::hardware_concurrency()))
        ->meta("INT")
        ->setter(options->nb_threads)
        ->checker(bc::check::is_number);
    }

    cmd->add_param("-h/--help", "show this message and exit.")
      ->as_flag()
      ->action(bc::Action::ShowHelp);
    cmd->add_param("--version", "show version and exit.")
      ->as_flag()
      ->action(bc::Action::ShowVersion);
    cmd->add_param("-v/--verbose", "verbosity level [debug|info|warning|error].")
      ->meta("STR")
      ->def("info")
      ->checker(bc::check::f::in("debug|info|warning|error"))
      ->setter(options->verbosity);
  }

}
