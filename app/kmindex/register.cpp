#include <iostream>

#include "register.hpp"

#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>

#include <spdlog/spdlog.h>

namespace kmq {

  kmq_options_t kmq_register_cli(parser_t parser, kmq_register_options_t options)
  {
    auto cmd = parser->add_command("register", "Register index.");

    cmd->add_param("-i/--global-index", "Global index path.")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("-n/--name", "Index name. (ignored with --from-file)") 
       ->meta("STR")
       ->def("")
       ->setter(options->index_name);

    cmd->add_param("-p/--index-path", "Index path (a kmtricks run). (ignored with --from-file)")
       ->meta("STR")
       ->def("")
       ->setter(options->index_path);

    cmd->add_param("-f/--from-file", "A tab-separated file with index_name<tab>index_path per line.")
       ->meta("STR")
       ->def("")
       ->setter(options->from_file)
       ->checker(bc::check::is_file);

    add_common_options(cmd, options);

    return options;
  }

  void main_register(kmq_options_t opt)
  {
    kmq_register_options_t o = std::static_pointer_cast<struct kmq_register_options>(opt);

    index i(o->global_index_path);

    if (o->from_file.empty() && (o->index_name.empty() || o->index_path.empty()))
    {
      throw kmq_error("With no --from-file, both --name and --index-path are required.");
    }

    if (!o->from_file.empty())
    {
      std::ifstream inf(o->from_file, std::ios::in);

      std::size_t line_no = 0;
      for (std::string line; std::getline(inf, line); line_no++)
      {
        auto parts = bc::utils::split(line, '\t');
        if (parts.size() != 2)
        {
          throw kmq_error(fmt::format("Invalid line: '{}'", line));
        }

        if (parts[0].empty())
        {
          throw kmq_error(fmt::format("Line {}: name is missing in '{}'", line_no, line));
        }
        
        if (parts[1].empty())
        {
          throw kmq_error(fmt::format("Line {}: path is missing in '{}'", line_no, line));
        }

        if (!fs::exists(fmt::format("{}/kmtricks.fof", parts[1])))
        {
          throw kmq_error(fmt::format("Line {}: '{}' is not a kmtricks index.", line_no, parts[1]));
        }

        if (i.has(parts[0]))
        {
          spdlog::warn("Line {}: name '{}' already exists, skipped.", line_no, parts[0]);
          continue;
        }

        spdlog::info("Register index '{}' as '{}'", parts[1], parts[0]);
        i.add_index(parts[0], parts[1]);
      }
    }
    else
    {
      i.add_index(o->index_name, o->index_path);
    }

    i.save();
  }
}
