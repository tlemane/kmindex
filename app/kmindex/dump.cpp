#include "dump.hpp"
#include <kmindex/index/index.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace kmq {

  kmq_options_t kmq_dump_cli(parser_t parser, kmq_dump_options_t options)
  {
    auto cmd = parser->add_command("dump", "Dump binary results.");

    auto is_kmq_index = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      return std::make_tuple(fs::exists(fmt::format("{}/index.json", v)),
                             bc::utils::format_error(p, v, fmt::format("'{}' is not an index.", v)));
    };

    cmd->add_param("-i/--index", "Global index path.")
       ->meta("STR")
       ->checker(bc::check::is_dir)
       ->checker(is_kmq_index)
       ->setter(options->global_index_path);

    auto format_setter = [options](const std::string& v) {
      options->format = str_to_format(v);
    };

    cmd->add_param("-f/--format", "Output format [json|yaml|matrix|json_vec|yaml_vec]")
       ->meta("STR")
       ->def("yaml")
       ->checker(bc::check::f::in("json|yaml|matrix|json_vec|yaml_vec"))
       ->setter_c(format_setter);

    cmd->add_param("--input", "Input file.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->setter(options->input);

    cmd->add_param("--output", "Output directory.")
       ->meta("STR")
       ->def("stdout")
       ->setter(options->output);

    add_common_options(cmd, options, false);

    return options;
  }

  void main_dump(kmq_options_t opt)
  {
    kmq_dump_options_t o = std::static_pointer_cast<struct kmq_dump_options>(opt);

    index global(o->global_index_path);

    std::string index_name = fs::path(o->input).stem();

    auto infos = global.get(index_name);


    std::ostream* outstream = nullptr;

    if (o->output == "stdout")
      outstream = &std::cout;
    else
    {
      std::string output_path = fmt::format("{}/{}.{}", o->output, index_name, extension(o->format));
      outstream = new std::ofstream(output_path, std::ios::out);
    }

    qres_reader qr(o->input);


    switch (o->format)
    {
      case format::yaml:
      case format::yaml_with_positions:
        qr.to_yaml(*outstream, infos.samples());
        break;
      case format::json:
      case format::json_with_positions:
      case format::matrix:
        throw kmq_error("Work in progress, dump supports only yaml output.");
    }

    if (outstream)
      delete outstream;
  }
}
