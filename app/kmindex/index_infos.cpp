#include "index_infos.hpp"

#include <iomanip>
#include <iostream>

#include <kmindex/utils.hpp>
#include <kmindex/index/index.hpp>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace kmq {

  kmq_options_t kmq_infos_cli(parser_t parser, kmq_infos_options_t options)
  {
    auto cmd = parser->add_command("index-infos", "Print index informations.");

    cmd->add_param("-i/--index", "global index path.")
       ->meta("STR")
       ->checker(bc::check::is_dir)
       ->setter(options->global_index_path);

    add_common_options(cmd, options, false);

    return options;
  }

  void main_infos(kmq_options_t opt)
  {
    kmq_infos_options_t o = std::static_pointer_cast<struct kmq_infos_options>(opt);

    auto mergeable = index(o->global_index_path).mergeable();

    std::ifstream inf(fmt::format("{}/index.json", o->global_index_path), std::ios::in);
    check_fstream_good(o->global_index_path, inf);
    json data = json::parse(inf);

    for (auto& v : mergeable)
      spdlog::info("[{}] are mergeable. See kmindex merge --help.", fmt::join(v, ","));

    std::cout << std::setw(4) << data << std::endl;
  }
}

