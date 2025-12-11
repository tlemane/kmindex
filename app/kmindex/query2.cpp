
#include "query2.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>

#include <kmindex/threadpool.hpp>
#include <kmindex/exceptions.hpp>
#include <kseq++/seqio.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <atomic_queue/atomic_queue.h>

namespace kmq {

  struct fastx_record {
    std::string name;
    std::string seq;
    fastx_record() noexcept {}
    fastx_record(std::string&& name, std::string&& seq) noexcept
      : name(std::move(name)), seq(std::move(seq)) {}
  };

  static constexpr bool minimize_contention = true;
  static constexpr bool maximize_throughput = true;
  static constexpr bool total_ordering = true;
  static constexpr bool spsc = false;
  static constexpr std::size_t queue_size = 2048;
  using queue_type = atomic_queue::AtomicQueue2<
    fastx_record, queue_size, minimize_contention, maximize_throughput, total_ordering, spsc
  >;

  kmq_options_t kmq_query2_cli(parser_t parser, kmq_query2_options_t options)
  {
    auto cmd = parser->add_command("query2", "Query index.");

    auto is_kmq_index = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      return std::make_tuple(fs::exists(fmt::format("{}/index.json", v)), bc::utils::format_error(p, v, fmt::format("'{}' is not an index.", v)));
    };

    cmd->add_param("-i/--index", "Global index path.")
       ->meta("STR")
       ->checker(bc::check::is_dir)
       ->checker(is_kmq_index)
       ->setter(options->global_index_path);

    auto name_setter = [options](const std::string& v) {
      if (v != "all")
        options->index_names = bc::utils::split(v, ',');
    };

    cmd->add_param("-n/--names", "Sub-indexes to query, comma separated.")
       ->meta("STR")
       ->def("all")
       ->setter_c(name_setter);

    cmd->add_param("-z/--zvalue", "Index s-mers and query (s+z)-mers (findere algorithm).")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::f::range(0, 8))
       ->setter(options->z);

    cmd->add_param("-r/--threshold", "Shared k-mers threshold. in [0.0, 1.0]")
       ->meta("FLOAT")
       ->def("0.0")
       ->checker(bc::check::f::range(0.0, 1.0))
       ->setter(options->sk_threshold);

    auto not_dir = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      return std::make_tuple(!fs::exists(v), bc::utils::format_error(p, v, "Directory already exists."));
    };

    cmd->add_param("-o/--output", "Output directory.")
       ->meta("STR")
       ->def("output")
       ->checker(not_dir)
       ->setter(options->output);

    cmd->add_param("-q/--fastx", "Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->checker(bc::check::f::ext(
         "fa|fq|fasta|fastq|fna|fa.gz|fq.gz|fasta.gz|fastq.gz|fna.gz|fa.bz2|fq.bz2|fasta.bz2|fastq.bz2|fna.bz2"))
       ->setter(options->input);

    auto format_setter = [options](const std::string& v) {
      options->format = str_to_format(v);
    };

    cmd->add_param("-f/--format", "Output format [json|matrix|json_vec|jsonl|jsonl_vec]")
       ->meta("STR")
       ->def("json")
       ->checker(bc::check::f::in("json|matrix|json_vec|jsonl|jsonl_vec"))
       ->setter_c(format_setter);

    cmd->add_param("--fast", "Keep more pages in cache (see doc for details).")
       ->as_flag()
       ->setter(options->cache);

    cmd->add_param("-u/--uncompressed", "Use uncompressed partitions (if available).")
       ->as_flag()
       ->hide()
       ->setter(options->uncompressed);


    add_common_options(cmd, options, true, 1);

    return options;
  }

  void main_query2(kmq_options_t opt)
  {
    kmq_query2_options_t o = std::static_pointer_cast<struct kmq_query2_options>(opt);

    Timer gtime;
    {
      spdlog::info("Loading global index: {}", o->global_index_path);
      Timer load_time;
      index global(o->global_index_path);
      spdlog::info("Global index loaded ({}).", load_time.formatted());
    }

    spdlog::info(
      "Global index: '{}'", fs::absolute(o->global_index_path + "/").parent_path().filename().string());

    if (o->index_names.empty())
    {
      o->index_names = global.all();
    }
    else
    {
      if (o->index_names[0][0] == '@')
      {
        std::ifstream fs(o->index_names[0].substr(1));
        o->index_names.clear();
        for (std::string line; std::getline(fs, line);)
        {
          o->index_names.push_back(line);
        }
      }
      spdlog::info("Sub-indexes to query: [{}]", fmt::join(o->index_names, ","));
    }

    for (const auto& name : o->index_names)
    {
      if (!global.has_index(name))
        throw kmq_error(fmt::format("{} subindex does not exist!", name));
    }

    if (!o->single.empty())
      spdlog::warn("--single-query: all query results are kept in memory");

    ThreadPool pool(opt->nb_threads);

    klibpp::SeqStreamIn iss(o->input.c_str());
    std::vector<klibpp::KSeq> records;
    klibpp::KSeq record;

    while (iss >> record)
    {
      records.push_back(record);
    }

    bool with_positions = o->format == format::json_with_positions || o->format == format::jsonl_with_positions;
    for (auto& index_name : o->index_names)
    {
      pool.add_task([&o, &global, &index_name, &records, with_positions](int i){
        Timer timer;
        auto infos = global.get(index_name);
        spdlog::info("Starting '{}' query ({} samples)", infos.name(), infos.nb_samples());

        batch_query b(infos.nb_samples(),
                      infos.nb_partitions(),
                      infos.smer_size(),
                      o->z,
                      infos.bw(),
                      infos.get_repartition(),
                      infos.get_hash_w(),
                      infos.minim_size()
        );

        for (auto& record : records)
          b.add_query(record.name, record.seq);

        if (o->uncompressed)
        {
          if (infos.has_uncompressed_partitions())
          {
            spdlog::info("Using uncompressed partitions for index '{}'.", index_name);
            infos.set_compress(false);
            auto fof_bak = fmt::format("{}/kmtricks.fof.bak", infos.get_directory());
            if (fs::exists(fof_bak))
              infos.use_fof(fof_bak);
          }
          else
          {
            spdlog::warn("Index '{}' has no uncompressed partitions, using compressed ones.", index_name);
          }
        }

        kindex ki(infos, o->cache);

        std::size_t nq = b.size();
        ki.solve_batch(b);
        b.free_smers();

        query_result_agg agg;
        for (auto&& r : b.response())
        {
          agg.add(query_result(std::move(r), o->z, infos, with_positions));
        }
        agg.output(infos, o->output, o->format, "", o->sk_threshold);

        spdlog::info("Index '{}' processed. ({})", infos.name(), timer.formatted());
      });
    }
    pool.join_all();
    spdlog::info("Done ({}).", gtime.formatted());
  }
}

