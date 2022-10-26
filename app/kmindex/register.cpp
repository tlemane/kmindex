#include <iostream>

#include "register.hpp"

#include <kmindex/index/index.hpp>

#include <spdlog/spdlog.h>

namespace kmq {

  kmq_options_t kmq_register_cli(parser_t parser, kmq_register_options_t options)
  {
    auto cmd = parser->add_command("register", "register a new index.");

    cmd->add_param("--name", "index name(s), comma separated")
       ->meta("STR")
       ->setter(options->index_name);

    cmd->add_param("--global-index", "global index path")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("--index", "index path")
       ->meta("STR")
       ->setter(options->index_path);

    add_common_options(cmd, options);

    return options;
  }

  void main_register(kmq_options_t opt)
  {
    kmq_register_options_t o = std::static_pointer_cast<struct kmq_register_options>(opt);

    index i(o->global_index_path);
    i.add_index(o->index_name, o->index_path);
    i.save();
  }
}
