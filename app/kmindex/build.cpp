#include "build.hpp"
#include "utils.hpp"
#include "common.hpp"

#include <kmindex/index/index.hpp>
#include <kmindex/version.hpp>
#include <kmindex/exceptions.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <semver.hpp>

namespace kmq {

  kmq_options_t kmq_build_cli(parser_t parser, kmq_build_options_t options)
  {
    auto cmd = parser->add_command("build", "Build index.");

    cmd->add_param("-i/--index", "Global index path.")
       ->meta("STR")
       ->setter(options->global_index_path);

    cmd->add_param("-f/--fof", "kmtricks input file.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->setter(options->fof);

    cmd->add_param("-d/--run-dir", "kmtricks runtime directory.")
       ->meta("STR")
       ->setter(options->directory);

    cmd->add_param("-r/--register-as", "Index name.")
       ->meta("STR")
       ->setter(options->name);

    cmd->add_param("--from", "Use parameters from a pre-registered index.")
       ->def("")
       ->meta("STR")
       ->setter(options->from);

    cmd->add_param("--no-check", "Bypass kmtricks version checking (hidden options)")
       ->as_flag()
       ->hide()
       ->setter(options->no_check);

    std::string km_path_help =
      "Path to kmtricks binary.\n"
      "                       - If empty, kmtricks is searched in $PATH and\n"
      "                         at the same location as the kmindex binary.";
    cmd->add_param("--km-path", km_path_help)
      ->def("")
      ->meta("STR")
      ->checker(bc::check::is_file)
      ->setter(options->km_path);

    auto kg = cmd->add_group("kmtricks parameters", "See kmtricks pipeline --help");
    kg->add_param("-k/--kmer-size", "Size of a k-mer. in [8, 31]")
       ->def("31")
       ->meta("INT")
       ->checker(bc::check::f::range(8, 31))
       ->setter(options->kmer_size);

    kg->add_param("-m/--minim-size", "Size of minimizers. in [4, 15]")
       ->def("10")
       ->meta("INT")
       ->checker(bc::check::f::range(4, 15))
       ->setter(options->minim_size);

    kg->add_param("--hard-min", "Min abundance to keep a k-mer.")
       ->def("2")
       ->meta("INT")
       ->setter(options->hard_min);

    kg->add_param("--nb-partitions", "Number of partitions (0=auto).")
       ->def("0")
       ->meta("INT")
       ->checker(bc::check::is_number)
       ->setter(options->nb_partitions);

    kg->add_param("--cpr", "Compress intermediate files.")
      ->as_flag()
      ->setter(options->cpr);

    auto bfm = cmd->add_group("presence/absence indexing", "");
    bfm->add_param("--bloom-size", "Bloom filter size.")
               ->def("")
               ->meta("INT")
               ->checker(bc::check::is_number)
               ->setter(options->bloom_size);

    auto bfcm = cmd->add_group("abundance indexing", "");
    bfcm->add_param("--nb-cell", "Number of cells in counting Bloom filter.")
               ->def("")
               ->meta("INT")
               ->checker(bc::check::is_number)
               ->setter(options->nb_cell);

    std::string bwidth_help =
      "Number of bits per cell. {2}\n"
      "                 - Abundances are indexed by log2 classes (nb classes = 2^{bitw})\n"
      "                   For example, using --bitw 3 resulting in the following classes:\n"
      "                     0 -> 0\n"
      "                     1 -> [1,2) \n"
      "                     2 -> [2,4) \n"
      "                     3 -> [4,8) \n"
      "                     4 -> [8,16) \n"
      "                     5 -> [16,32) \n"
      "                     6 -> [32,64) \n"
      "                     7 -> [64,max(uint32_t))";
    bfcm->add_param("--bitw", bwidth_help)
        ->def("")
        ->meta("INT")
        ->setter(options->bw);

    // Currently not used
    bfcm->add_param("--class", "")
         ->def("log2")
         ->meta("STR")
         ->hide();

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
      "pipeline --file {} --run-dir {} --kmer-size {} --hard-min {} "
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

    if (opt->bw > 1)
      fmt_cmd += fmt::format("--bitw {} --mode hash:bfc:bin ", opt->bw);
    else
      fmt_cmd += "--mode hash:bf:bin ";


    if (!opt->from.empty())
      fmt_cmd += fmt::format("--repart-from {} ", opt->from);

    if (opt->cpr)
      fmt_cmd += "--cpr ";

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
        throw kmq_error(
          fmt::format("kmindex v{} requires kmtricks >= v{}, but found v{}.",
                      kmindex_version.to_string(), min_kmv_required.to_string(), kmv.to_string()
          )
        );
      }
      if ((kmv < semver::version("1.4.0")) && options->cpr)
      {
        spdlog::warn("--cpr ignored. Requires kmtricks >= v1.4.0, found v{}.", kmv.to_string());
        options->cpr = false;
      }
    }

    if (!options->bloom_size && !options->nb_cell)
      throw kmq_error("--bloom-size or --nb-cell must be provided.");

    if (options->bloom_size && options->nb_cell)
      throw kmq_error("--bloom-size and --nb-cell are mutually exclusive. Use --bloom-size for presence/absence indexing OR --nb-cell for log abundance indexing.");

    if (options->bloom_size > 0)
      options->bw = 1;

    options->bloom_size = std::max(options->bloom_size, options->nb_cell);

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
