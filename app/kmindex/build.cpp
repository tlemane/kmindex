#include "build.hpp"
#include "utils.hpp"
#include "common.hpp"

#include <kmindex/index/index.hpp>
#include <kmindex/version.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <semver.hpp>

namespace kmq {

  kmq_options_t kmq_build_cli(parser_t parser, kmq_build_options_t options)
  {
    auto cmd = parser->add_command("build", "Build index.");

    cmd->add_param("-i/--index", "global index path.")
       ->meta("STR")
       ->setter(options->global_index_path);

    cmd->add_param("-f/--fof", "kmtricks input file.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->setter(options->fof);

    cmd->add_param("-d/--run-dir", "kmtricks runtime directory.")
       ->meta("STR")
       ->setter(options->directory);

    cmd->add_param("-r/--register-as", "index name.")
       ->meta("STR")
       ->setter(options->name);

    cmd->add_param("--from", "use parameters from a pre-registered index.")
       ->def("")
       ->meta("STR")
       ->setter(options->from);

    cmd->add_param("--no-check", "bypass kmtricks version checking (hidden options)")
       ->as_flag()
       ->hide()
       ->setter(options->no_check);

    std::string km_path_help =
      "path to kmtricks binary.\n"
      "                   - If empty, kmtricks is searched in $PATH and\n"
      "                     at the same location as kmindex binary.";
    cmd->add_param("--km-path", km_path_help)
      ->def("")
      ->meta("STR")
      ->checker(bc::check::is_file)
      ->setter(options->km_path);

    auto kg = cmd->add_group("kmtricks parameters", "See kmtricks pipeline --help");
    kg->add_param("-k/--kmer-size", "size of a k-mer. in [8, 31]")
       ->def("31")
       ->meta("INT")
       ->checker(bc::check::f::range(8, 31))
       ->setter(options->kmer_size);

    kg->add_param("-m/--minim-size", "size of minimizers. in [4, 15]")
       ->def("10")
       ->meta("INT")
       ->checker(bc::check::f::range(4, 15))
       ->setter(options->minim_size);

    kg->add_param("--hard-min", "min abundance to keep a k-mer.")
       ->def("2")
       ->meta("INT");

    kg->add_param("--bloom-size", "bloom filter size.")
       ->def("10000000")
       ->meta("INT")
       ->setter(options->bloom_size);

    kg->add_param("--nb-partitions", "number of partitions (0=auto).")
       ->def("0")
       ->meta("INT")
       ->checker(bc::check::is_number)
       ->setter(options->nb_partitions);

    add_common_options(cmd, options, true);

    return options;
  }

  semver::version get_kmtricks_version(std::string kmpath)
  {
    char buf[256];
    std::string version_str;
    kmpath += " --version  2>&1 >/dev/null";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(kmpath.c_str(), "r"), pclose);

    if (!pipe)
      throw std::runtime_error("popen() failed.");

    while (fgets(buf, sizeof(buf), pipe.get()) != nullptr)
      version_str += buf;

    return semver::version(bc::utils::trim(version_str.substr(10)));
  }

  void param_from(kmq_build_options_t opt)
  {
    index global(opt->global_index_path);
    auto infos = global.get(opt->from);

    opt->bloom_size = infos.bloom_size();
    opt->kmer_size = infos.smer_size();
    opt->minim_size = infos.minim_size();
  }

  std::string get_kmtricks_cmd(kmq_build_options_t opt)
  {
    static std::string cmd_template =
      "pipeline --file {} --run-dir {} --kmer-size {} --hard-min {} --mode hash:bf:bin "
      "--bloom-size {} --minimizer-size {} --nb-partitions {} --threads {} ";

    if (!opt->from.empty())
      param_from(opt);

    std::string fmt_cmd = fmt::format(
      cmd_template,
      opt->fof,
      opt->directory,
      opt->kmer_size,
      opt->hard_min,
      opt->bloom_size,
      opt->minim_size,
      opt->nb_partitions,
      opt->nb_threads
    );

    if (!opt->from.empty())
      fmt_cmd += fmt::format("--repart-from {}", opt->from);

    return fmt_cmd;
  }

  void execute_kmtricks(kmq_build_options_t opt, const std::string& km_cmd)
  {
    auto cmd = get_kmtricks_cmd(opt);
    exec_cmd(km_cmd, cmd);
  }

  void register_index(kmq_build_options_t opt)
  {
    index i(opt->global_index_path);
    i.add_index(opt->name, opt->directory);
    i.save();
  }

  void main_build(kmq_options_t opt)
  {
    kmq_build_options_t options = std::static_pointer_cast<struct kmq_build_options>(opt);

    std::string km_cmd = cmd_exists(
        options->km_path.empty()
          ? get_binary_dir()
          : fs::path(options->km_path).parent_path().string(),
        "kmtricks"
    );

    semver::version kmv = get_kmtricks_version(km_cmd);

    if (!options->no_check)
    {
      if (!(kmv >= min_kmv_required))
      {
        throw std::runtime_error(
          fmt::format("kmindex v{} requires kmtricks >= v{}, but found v{}.",
                      kmindex_version.to_string(), min_kmv_required.to_string(), kmv.to_string()
          )
        );
      }
    }

    spdlog::info("Found kmtricks v{}.", kmv.to_string());

    if (options->from.empty())
      spdlog::info("Build index '{}'...", options->name);
    else
      spdlog::info("Build index '{}' using '{}' parameters...", options->name, options->from);

    execute_kmtricks(options, km_cmd);

    spdlog::info("Register index '{}'...", options->name);
    register_index(options);
  }
}
